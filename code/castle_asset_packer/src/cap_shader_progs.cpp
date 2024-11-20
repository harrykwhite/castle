#include <iostream>
#include <castle_common/cc_misc.h>
#include "cap_shared.h"

struct s_shader_prog_pack_info
{
    std::string vs_file_path_end;
    std::string fs_file_path_end;
};

const s_shader_prog_pack_info k_shader_prog_pack_infos[] = {
    {"/shaders/sprite_quad.vert", "/shaders/sprite_quad.frag"},
    {"/shaders/char_quad.vert", "/shaders/char_quad.frag"}
};

const int k_shader_prog_cnt = CC_STATIC_ARRAY_LEN(k_shader_prog_pack_infos);

bool pack_shader_progs(std::ofstream &assets_file_ofs, const std::string &assets_dir)
{
    for (auto &pack_info : k_shader_prog_pack_infos)
    {
        // Pack vertex shader then fragment shader.
        for (int i = 0; i < 2; i++)
        {
            const std::string &shader_file_path_end = i == 0 ? pack_info.vs_file_path_end : pack_info.fs_file_path_end;
            const std::string shader_file_path = assets_dir + shader_file_path_end;

            // Open the shader file.
            std::ifstream shader_ifs(shader_file_path, std::ios::binary | std::ios::ate);

            if (!shader_ifs)
            {
                std::cout << "ERROR: Failed to open " << shader_file_path_end << "!" << std::endl;
                return false;
            }

            // Get the shader file size.
            const int shader_file_size = shader_ifs.tellg();
            shader_ifs.seekg(0, std::ios::beg);

            // Write shader source size to assets file.
            const int shader_src_size = shader_file_size + 1; // Account for the '\0' we're going to write.
            assets_file_ofs.write(reinterpret_cast<const char *>(&shader_src_size), sizeof(shader_src_size));

            // Allocate buffer for shader content and read it.
            std::unique_ptr<char[]> shader_src(new char[shader_src_size]);

            if (!shader_src)
            {
                std::cout << "ERROR: Failed to allocate " << shader_src_size << " bytes for a shader source!" << std::endl;
                return false;
            }

            shader_ifs.read(shader_src.get(), shader_file_size);
            shader_src[shader_src_size - 1] = '\0';

            // Write the shader content to assets file.
            assets_file_ofs.write(shader_src.get(), shader_src_size);

            std::cout << "Successfully packed shader with file path \"" << shader_file_path << "\"." << std::endl;
        }
    }

    return true;
}
