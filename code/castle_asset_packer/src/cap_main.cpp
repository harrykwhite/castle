#include <iostream>
#include "cap_shared.h"

constexpr int k_assets_file_path_buf_size = 128;

int main(const int arg_cnt, const char *const *const args)
{
    if (arg_cnt != 3)
    {
        std::cout << "ERROR: An assets directory and an assets file path must both be provided as command-line arguments!" << std::endl;
        return 1;
    }

    const std::string assets_dir = args[1];
    const std::string assets_file_path = args[2];

    // Open the assets file.
    std::ofstream assets_file_ofs(assets_file_path, std::ios::binary);

    if (!assets_file_ofs)
    {
        std::cout << "ERROR: Failed to open assets file with path \"" << assets_file_path << "\"!" << std::endl;
        return 1;
    }

    // Write asset counts to the file header.
    assets_file_ofs.write(reinterpret_cast<const char *>(&k_tex_cnt), sizeof(k_tex_cnt));
    assets_file_ofs.write(reinterpret_cast<const char *>(&k_shader_prog_cnt), sizeof(k_shader_prog_cnt));
    assets_file_ofs.write(reinterpret_cast<const char *>(&k_font_cnt), sizeof(k_font_cnt));

    // Pack textures and shader programs into assets file.
    if (!pack_textures(assets_file_ofs, assets_dir)
        || !pack_shader_progs(assets_file_ofs, assets_dir)
        || !pack_fonts(assets_file_ofs, assets_dir))
    {
        std::remove(assets_file_path.c_str());
        return 1;
    }

    return 0;
}
