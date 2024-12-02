#include <stb_image.h>
#include "cap_shared.h"

static const char *const ik_texFilePathEnds[] = {
    "\\textures\\player_ent.png",
    "\\textures\\enemy_ent.png",
    "\\textures\\sword.png",
    "\\textures\\tiles\\dirt.png",
    "\\textures\\tiles\\stone.png",
    "\\textures\\ui\\inv_slot.png",
    "\\textures\\ui\\cursor.png"
};

static_assert(cc::VANILLA_TEX_CNT == CC_STATIC_ARRAY_LEN(ik_texFilePathEnds));

bool pack_textures(FILE *const assetFileStream, const char *const assetsDir)
{
    for (const char *const texFilePathEnd : ik_texFilePathEnds)
    {
        // Determine the texture file path.
        char texFilePath[gk_assetFilePathMaxLen + 1];
        snprintf(texFilePath, sizeof(texFilePath), "%s%s", assetsDir, texFilePathEnd);

        // Load and write texture data.
        cc::Vec2DInt texSize;
        stbi_uc *const pxData = stbi_load(texFilePath, &texSize.x, &texSize.y, NULL, cc::gk_texChannelCnt);

        if (!pxData)
        {
            cc::log_error("%s", stbi_failure_reason());
            return false;
        }

        if (texSize.x > cc::gk_texSizeLimit.x || texSize.y > cc::gk_texSizeLimit.y)
        {
            cc::log_error("The size of the texture with file path \"%s\" exceeds the limit of %d by %d!", texFilePath, cc::gk_texSizeLimit.x, cc::gk_texSizeLimit.y);
            stbi_image_free(pxData);
            return false;
        }

        fwrite(&texSize, sizeof(texSize), 1, assetFileStream);
        fwrite(pxData, texSize.x * texSize.y * cc::gk_texChannelCnt, 1, assetFileStream);

        stbi_image_free(pxData);

        cc::log("Successfully packed texture with file path \"%s\".", texFilePath);
    }

    return true;
}
