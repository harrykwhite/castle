#include <iostream>
#include <stb_image.h>
#include <castle_common/cc_math.h>
#include <castle_common/cc_assets.h>
#include <castle_common/cc_misc.h>
#include "cap_shared.h"

const std::string k_tex_file_path_ends[] = {
    "/textures/characters/player.png",
    "/textures/tiles/dirt.png",
    "/textures/tiles/stone.png",
    "/textures/ui/inv_slot.png",
    "/textures/ui/cursor.png"
};

const int k_tex_cnt = CC_STATIC_ARRAY_LEN(k_tex_file_path_ends);

bool pack_textures(std::ofstream &assets_file_ofs, const std::string &assets_dir)
{
    for (int i = 0; i < k_tex_cnt; i++)
    {
        const std::string tex_file_path = assets_dir + k_tex_file_path_ends[i];

        cc::s_vec_2d_i tex_size;
        stbi_uc *const px_data = stbi_load(tex_file_path.c_str(), &tex_size.x, &tex_size.y, nullptr, cc::k_tex_channel_cnt);

        if (!px_data)
        {
            std::cout << "STB ERROR: " << stbi_failure_reason() << std::endl;
            return false;
        }

        if (tex_size.x > cc::k_tex_size_limit.x || tex_size.y > cc::k_tex_size_limit.y)
        {
            std::cout << "ERROR: The width or height of the source texture with file path \"" << tex_file_path << "\" exceeds the limit of " << cc::k_tex_size_limit.x << "x" << cc::k_tex_size_limit.y << "!" << std::endl;
            stbi_image_free(px_data);
            return false;
        }

        assets_file_ofs.write(reinterpret_cast<const char *>(&tex_size), sizeof(tex_size));
        assets_file_ofs.write(reinterpret_cast<const char *>(px_data), tex_size.x * tex_size.y * cc::k_tex_channel_cnt);

        stbi_image_free(px_data);

        std::cout << "Successfully packed texture with file path \"" << tex_file_path << "\"." << std::endl;
    }

    return true;
}
