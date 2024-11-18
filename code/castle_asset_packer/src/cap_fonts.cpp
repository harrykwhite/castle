#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <utility>
#include <castle_common/cc_math.h>
#include <castle_common/cc_misc.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "cap_shared.h"

struct s_font_pack_info
{
    std::string file_path_end;
    int pt_size;
};

constexpr int k_font_char_range_begin = 32;
constexpr int k_font_char_range_size = 95;
constexpr int k_font_tex_width_max = 1024;
constexpr int k_font_tex_channel_cnt = 4;

struct s_font_char_data
{
    int hor_offsets[k_font_char_range_size];
    int ver_offsets[k_font_char_range_size];
    int hor_advances[k_font_char_range_size];

    cc::s_rect src_rects[k_font_char_range_size];

    int kernings[k_font_char_range_size * k_font_char_range_size];
};

struct s_font_data
{
    int line_height;

    s_font_char_data chars;

    cc::s_vec_2d_i tex_size;
    std::unique_ptr<cc::u_byte[]> tex_px_data;
};

const s_font_pack_info k_font_pack_infos[] = {
    {"/fonts/eb_garamond.ttf", 18},
    {"/fonts/eb_garamond.ttf", 24},
    {"/fonts/eb_garamond.ttf", 36},
    {"/fonts/eb_garamond.ttf", 48}
};

const int k_font_cnt = CC_STATIC_ARRAY_LEN(k_font_pack_infos);

static inline int get_line_height(const FT_Face ft_face)
{
    return ft_face->size->metrics.height >> 6;
}

static int calc_largest_bitmap_width(const FT_Face ft_face, const FT_Library ft_lib)
{
    int width = 0;

    for (int i = 0; i < k_font_char_range_size; i++)
    {
        FT_Load_Glyph(ft_face, FT_Get_Char_Index(ft_face, k_font_char_range_begin + i), FT_LOAD_DEFAULT);
        FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_NORMAL);

        if (ft_face->glyph->bitmap.width > width)
        {
            width = ft_face->glyph->bitmap.width;
        }
    }

    return width;
}

static cc::s_vec_2d_i calc_font_tex_size(const FT_Face ft_face, const FT_Library ft_lib)
{
    const int largest_glyph_bitmap_width = calc_largest_bitmap_width(ft_face, ft_lib);
    const int ideal_tex_width = largest_glyph_bitmap_width * k_font_char_range_size;

    return {
        std::min(ideal_tex_width, k_font_tex_width_max),
        get_line_height(ft_face) * ((ideal_tex_width / k_font_tex_width_max) + 1)
    };
}

static std::unique_ptr<s_font_data> get_font_data(const std::string &file_path, const int pt_size, const FT_Library ft_lib)
{
    auto font_data = std::make_unique<s_font_data>();

    // Set up the font face.
    FT_Face ft_face;

    if (FT_New_Face(ft_lib, file_path.c_str(), 0, &ft_face))
    {
        std::cout << "ERROR: Failed to create a FreeType face object for font with file path \"" << file_path << "\"." << std::endl;
        return nullptr;
    }

    FT_Set_Char_Size(ft_face, pt_size << 6, 0, 96, 0);

    const int line_height = get_line_height(ft_face);

    // Initialise the font texture, setting all the pixels to transparent white.
    const cc::s_vec_2d_i tex_size = calc_font_tex_size(ft_face, ft_lib);
    const int tex_px_data_size = tex_size.x * tex_size.y * k_font_tex_channel_cnt;
    const auto tex_px_data = std::make_unique<cc::u_byte[]>(tex_px_data_size);

    for (int i = 0; i < tex_size.x * tex_size.y; ++i)
    {
        const int px_data_index = i * k_font_tex_channel_cnt;

        tex_px_data[px_data_index + 0] = 255;
        tex_px_data[px_data_index + 1] = 255;
        tex_px_data[px_data_index + 2] = 255;
        tex_px_data[px_data_index + 3] = 0;
    }

    // Get and store information for all font characters.
    int char_draw_x = 0;
    int char_draw_y = 0;

    for (int i = 0; i < k_font_char_range_size; i++)
    {
        const FT_UInt ft_char_index = FT_Get_Char_Index(ft_face, k_font_char_range_begin + i);

        FT_Load_Glyph(ft_face, ft_char_index, FT_LOAD_DEFAULT);
        FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_NORMAL);

        if (char_draw_x + ft_face->glyph->bitmap.width > k_font_tex_width_max)
        {
            char_draw_x = 0;
            char_draw_y += line_height;
        }

        font_data->chars.hor_offsets[i] = ft_face->glyph->metrics.horiBearingX >> 6;
        font_data->chars.ver_offsets[i] = (ft_face->size->metrics.ascender - ft_face->glyph->metrics.horiBearingY) >> 6;

        font_data->chars.hor_advances[i] = ft_face->glyph->metrics.horiAdvance >> 6;

        font_data->chars.src_rects[i].x = char_draw_x;
        font_data->chars.src_rects[i].y = char_draw_y;
        font_data->chars.src_rects[i].width = ft_face->glyph->bitmap.width;
        font_data->chars.src_rects[i].height = ft_face->glyph->bitmap.rows;

        // Set kernings (one per character combination).
        for (int j = 0; j < k_font_char_range_size; j++)
        {
            FT_Vector ft_kerning;
            FT_Get_Kerning(ft_face, FT_Get_Char_Index(ft_face, k_font_char_range_begin + j), ft_char_index, FT_KERNING_DEFAULT, &ft_kerning);

            font_data->chars.kernings[(k_font_char_range_size * i) + j] = ft_kerning.x >> 6;
        }

        // Update the font texture's pixel data with the character.
        for (int y = 0; y < font_data->chars.src_rects[i].height; y++)
        {
            for (int x = 0; x < font_data->chars.src_rects[i].width; x++)
            {
                const unsigned char px_alpha = ft_face->glyph->bitmap.buffer[(y * ft_face->glyph->bitmap.width) + x];

                if (px_alpha > 0)
                {
                    const int px_data_x = (font_data->chars.src_rects[i].x + x) * k_font_tex_channel_cnt;
                    const int px_data_index = ((font_data->chars.src_rects[i].y + y) * tex_size.x * k_font_tex_channel_cnt) + px_data_x;
                    tex_px_data[px_data_index + 3] = px_alpha;
                }
            }
        }

        char_draw_x += font_data->chars.src_rects[i].width;
    }

    FT_Done_Face(ft_face);

    return font_data;
}

bool pack_fonts(std::ofstream &assets_file_ofs, const std::string &assets_dir)
{
    FT_Library ft_lib;

    if (FT_Init_FreeType(&ft_lib))
    {
        std::cout << "ERROR: Failed to initialise FreeType!" << std::endl;
        return false;
    }

    for (auto &pack_info : k_font_pack_infos)
    {
        const std::string font_file_path = assets_dir + pack_info.file_path_end;

        // Get the font data using FreeType.
        std::unique_ptr<s_font_data> font_data = get_font_data(font_file_path, pack_info.pt_size, ft_lib);

        if (!font_data)
        {
            FT_Done_FreeType(ft_lib);
            return false;
        }

        // Write the font data to the file.
        assets_file_ofs.write(reinterpret_cast<const char *>(&font_data->line_height), sizeof(font_data->line_height));
        assets_file_ofs.write(reinterpret_cast<const char *>(&font_data->chars), sizeof(font_data->chars));

        assets_file_ofs.write(reinterpret_cast<const char *>(&font_data->tex_size), sizeof(font_data->tex_size));

        assets_file_ofs.write(reinterpret_cast<const char *>(font_data->tex_px_data.get()), font_data->tex_size.x * font_data->tex_size.y * k_font_tex_channel_cnt);

        std::cout << "Successfully packed font with file path \"" << font_file_path << "\" and point size " << pack_info.pt_size << "." << std::endl;
    }

    FT_Done_FreeType(ft_lib);

    return true;
}
