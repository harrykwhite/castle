#include <algorithm>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "cap_shared.h"

static constexpr int ik_fontCharHorPadding = 32;

struct FontPackingInfo
{
    const char *filePathEnd;
    int ptSize;
};

static constexpr FontPackingInfo ik_fontPackingInfos[] = {
    {"\\fonts\\eb_garamond.ttf", 18},
    {"\\fonts\\eb_garamond.ttf", 24},
    {"\\fonts\\eb_garamond.ttf", 36},
    {"\\fonts\\eb_garamond.ttf", 72}
};

static_assert(cc::CORE_FONT_CNT == CC_STATIC_ARRAY_LEN(ik_fontPackingInfos));

struct FontDisplayInfoWithTexPixels
{
    cc::FontDisplayInfo info;
    cc::Byte texPxData[cc::gk_texChannelCnt * cc::gk_texSizeLimit.x * cc::gk_texSizeLimit.y];
};

static inline int get_line_height(const FT_Face ftFace)
{
    return ftFace->size->metrics.height >> 6;
}

static int calc_largest_char_width(const FT_Face ftFace, const FT_Library ftLib)
{
    int width = 0;

    for (int i = 0; i < cc::gk_fontCharRangeSize; i++)
    {
        FT_Load_Glyph(ftFace, FT_Get_Char_Index(ftFace, cc::gk_fontCharRangeBegin + i), FT_LOAD_DEFAULT);
        FT_Render_Glyph(ftFace->glyph, FT_RENDER_MODE_NORMAL);

        if (ftFace->glyph->bitmap.width > width)
        {
            width = ftFace->glyph->bitmap.width;
        }
    }

    return width;
}

static cc::Vec2DInt calc_font_tex_size(const FT_Face ftFace, const FT_Library ftLib)
{
    const int largestCharWidth = calc_largest_char_width(ftFace, ftLib);
    const int idealTexWidth = ik_fontCharHorPadding + ((largestCharWidth + ik_fontCharHorPadding) * cc::gk_fontCharRangeSize);

    return {
        std::min(idealTexWidth, cc::gk_texSizeLimit.x),
        get_line_height(ftFace) * static_cast<int>(ceilf(static_cast<float>(idealTexWidth) / cc::gk_texSizeLimit.x))
    };
}

static bool init_font_display_info_and_tex_pixels(FontDisplayInfoWithTexPixels *const infoWithPixels, const char *const filePath, const int ptSize, const FT_Library ftLib)
{
    // Set up the font face.
    FT_Face ftFace;

    if (FT_New_Face(ftLib, filePath, 0, &ftFace))
    {
        cc::log_error("Failed to create a FreeType face object for font with file path \"%s\".", filePath);
        return false;
    }

    FT_Set_Char_Size(ftFace, ptSize << 6, 0, 96, 0);

    infoWithPixels->info.lineHeight = get_line_height(ftFace);

    // Initialise the font texture, setting all the pixels to transparent white.
    infoWithPixels->info.texSize = calc_font_tex_size(ftFace, ftLib);

    if (infoWithPixels->info.texSize.y > cc::gk_texSizeLimit.y)
    {
        cc::log_error("Font texture size is too large!");
        return false;
    }

    const int texPxDataSize = infoWithPixels->info.texSize.x * infoWithPixels->info.texSize.y * cc::gk_texChannelCnt;

    for (int i = 0; i < infoWithPixels->info.texSize.x * infoWithPixels->info.texSize.y; ++i)
    {
        const int pxDataIndex = i * cc::gk_texChannelCnt;

        infoWithPixels->texPxData[pxDataIndex + 0] = 255;
        infoWithPixels->texPxData[pxDataIndex + 1] = 255;
        infoWithPixels->texPxData[pxDataIndex + 2] = 255;
        infoWithPixels->texPxData[pxDataIndex + 3] = 0;
    }

    // Get and store information for all font characters.
    int charDrawX = ik_fontCharHorPadding;
    int charDrawY = 0;

    for (int i = 0; i < cc::gk_fontCharRangeSize; i++)
    {
        const FT_UInt ftCharIndex = FT_Get_Char_Index(ftFace, cc::gk_fontCharRangeBegin + i);

        FT_Load_Glyph(ftFace, ftCharIndex, FT_LOAD_DEFAULT);
        FT_Render_Glyph(ftFace->glyph, FT_RENDER_MODE_NORMAL);

        if (charDrawX + ftFace->glyph->bitmap.width + ik_fontCharHorPadding > cc::gk_texSizeLimit.x)
        {
            charDrawX = ik_fontCharHorPadding;
            charDrawY += infoWithPixels->info.lineHeight;
        }

        infoWithPixels->info.chars.horOffsets[i] = ftFace->glyph->metrics.horiBearingX >> 6;
        infoWithPixels->info.chars.verOffsets[i] = (ftFace->size->metrics.ascender - ftFace->glyph->metrics.horiBearingY) >> 6;

        infoWithPixels->info.chars.horAdvances[i] = ftFace->glyph->metrics.horiAdvance >> 6;

        infoWithPixels->info.chars.srcRects[i].x = charDrawX;
        infoWithPixels->info.chars.srcRects[i].y = charDrawY;
        infoWithPixels->info.chars.srcRects[i].width = ftFace->glyph->bitmap.width;
        infoWithPixels->info.chars.srcRects[i].height = ftFace->glyph->bitmap.rows;

        // Set kernings (one per character combination).
        for (int j = 0; j < cc::gk_fontCharRangeSize; j++)
        {
            FT_Vector ftKerning;
            FT_Get_Kerning(ftFace, FT_Get_Char_Index(ftFace, cc::gk_fontCharRangeBegin + j), ftCharIndex, FT_KERNING_DEFAULT, &ftKerning);

            infoWithPixels->info.chars.kernings[(cc::gk_fontCharRangeSize * i) + j] = ftKerning.x >> 6;
        }

        // Update the font texture's pixel data with the character.
        for (int y = 0; y < infoWithPixels->info.chars.srcRects[i].height; y++)
        {
            for (int x = 0; x < infoWithPixels->info.chars.srcRects[i].width; x++)
            {
                const unsigned char pxAlpha = ftFace->glyph->bitmap.buffer[(y * ftFace->glyph->bitmap.width) + x];

                if (pxAlpha > 0)
                {
                    const int pxX = infoWithPixels->info.chars.srcRects[i].x + x;
                    const int pxY = infoWithPixels->info.chars.srcRects[i].y + y;
                    const int pxDataIndex = (pxY * infoWithPixels->info.texSize.x * cc::gk_texChannelCnt) + (pxX * cc::gk_texChannelCnt);

                    infoWithPixels->texPxData[pxDataIndex + 3] = pxAlpha;
                }
            }
        }

        charDrawX += infoWithPixels->info.chars.srcRects[i].width + ik_fontCharHorPadding;
    }

    FT_Done_Face(ftFace);

    return true;
}

bool pack_fonts(FILE *const assetFileStream, const char *const assetsDir, cc::MemArena &memArena)
{
    // Set up FreeType.
    FT_Library ftLib;

    if (FT_Init_FreeType(&ftLib))
    {
        cc::log_error("Failed to initialise FreeType!");
        return false;
    }

    // Reserve memory for font display information and texture pixels (reused for every font).
    const auto fontDisplayInfoWithTexPixels = cc::push_to_mem_arena<FontDisplayInfoWithTexPixels>(memArena);

    for (const FontPackingInfo &packingInfo : ik_fontPackingInfos)
    {
        // Determine the font file path.
        char fontFilePath[gk_assetFilePathMaxLen + 1];
        snprintf(fontFilePath, sizeof(fontFilePath), "%s%s", assetsDir, packingInfo.filePathEnd);

        // Load the font data using FreeType.
        if (!init_font_display_info_and_tex_pixels(fontDisplayInfoWithTexPixels, fontFilePath, packingInfo.ptSize, ftLib))
        {
            FT_Done_FreeType(ftLib);
            return false;
        }

        // Write the font data to the file.
        fwrite(fontDisplayInfoWithTexPixels, sizeof(*fontDisplayInfoWithTexPixels), 1, assetFileStream);

        // Clear the font data for next time (more for ease of debugging).
        memset(fontDisplayInfoWithTexPixels, 0, sizeof(*fontDisplayInfoWithTexPixels));

        cc::log("Successfully packed font with file path \"%s\" and point size %d.", fontFilePath, packingInfo.ptSize);
    }

    FT_Done_FreeType(ftLib);

    return true;
}
