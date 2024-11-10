#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <stb_image.h>
#include <castle_common/cc_assets.h>
#include <castle_common/cc_debugging.h>
#include <castle_common/cc_math.h>
#include <castle_common/cc_misc.h>

struct s_src_shader_prog_info
{
    std::string vs_file_path_end;
    std::string fs_file_path_end;
};

constexpr int k_src_asset_file_path_buf_size = 128;

const std::string k_src_tex_file_path_ends[] = {
    "/textures/characters/player.png",
    "/textures/tiles/dirt.png",
    "/textures/tiles/stone.png"
};

const s_src_shader_prog_info k_src_shader_prog_infos[] = {
    {"/shaders/sprite_quad.vert", "/shaders/sprite_quad.frag"}
};

constexpr int k_src_tex_cnt = CC_STATIC_ARRAY_LEN(k_src_tex_file_path_ends);
constexpr int k_src_shader_prog_cnt = CC_STATIC_ARRAY_LEN(k_src_shader_prog_infos);

static bool pack_textures(std::ofstream &assets_file_ofs, std::string &src_asset_file_path, const int src_assets_dir_len)
{
    for (int i = 0; i < k_src_tex_cnt; i++)
    {
        // Update the asset file path with the current texture path.
        src_asset_file_path.replace(src_assets_dir_len, std::string::npos, k_src_tex_file_path_ends[i]);

        // Load and write the texture.
        cc::s_vec_2d_int tex_size;
        stbi_uc *const px_data = stbi_load(src_asset_file_path.c_str(), &tex_size.x, &tex_size.y, nullptr, cc::k_tex_channel_cnt);

        if (!px_data)
        {
            std::cout << "ERROR: " << stbi_failure_reason() << std::endl;
            return false;
        }

        if (tex_size.x > cc::k_tex_size_limit.x || tex_size.y > cc::k_tex_size_limit.y)
        {
            std::cout << "ERROR: The width or height of the source texture with file path \"" << src_asset_file_path << "\" exceeds the limit of " << cc::k_tex_size_limit.x << "x" << cc::k_tex_size_limit.y << "!" << std::endl;
            stbi_image_free(px_data);
            return false;
        }

        assets_file_ofs.write(reinterpret_cast<const char *>(&tex_size), sizeof(tex_size));
        assets_file_ofs.write(reinterpret_cast<const char *>(px_data), tex_size.x * tex_size.y * cc::k_tex_channel_cnt);

        stbi_image_free(px_data);

        std::cout << "Successfully packed texture with file path \"" << src_asset_file_path << "\"." << std::endl;
    }

    return true;
}

static bool pack_shader_progs(std::ofstream &assets_file_ofs, std::string &src_asset_file_path, const int src_assets_dir_len)
{
    for (int i = 0; i < k_src_shader_prog_cnt; i++)
    {
        // Pack vertex shader then fragment shader.
        for (int j = 0; j < 2; j++)
        {
            const std::string &src_shader_file_path_end = j == 0 ? k_src_shader_prog_infos[i].vs_file_path_end : k_src_shader_prog_infos[i].fs_file_path_end;
            src_asset_file_path.replace(src_assets_dir_len, std::string::npos, src_shader_file_path_end);

            std::ifstream shader_ifs(src_asset_file_path, std::ios::binary | std::ios::ate);

            if (!shader_ifs)
            {
                std::cout << "ERROR: Failed to open " << src_shader_file_path_end << "!" << std::endl;
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

            std::cout << "Successfully packed shader with file path \"" << src_asset_file_path << "\"." << std::endl;
        }
    }

    return true;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "ERROR: An assets source directory and an output assets file path must both be provided as command-line arguments!" << std::endl;
        return 1;
    }

    std::string src_asset_file_path(argv[1]);
    src_asset_file_path.reserve(k_src_asset_file_path_buf_size);

    const int src_assets_dir_len = src_asset_file_path.size();
    const std::string assets_file_path = argv[2];

    std::ofstream assets_file_ofs(assets_file_path, std::ios::binary);

    if (!assets_file_ofs)
    {
        std::cout << "ERROR: Failed to open assets file with path \"" << assets_file_path << "\"!" << std::endl;
        return 1;
    }

    // Write texture and shader program counts to the assets file header.
    assets_file_ofs.write(reinterpret_cast<const char *>(&k_src_tex_cnt), sizeof(k_src_tex_cnt));
    assets_file_ofs.write(reinterpret_cast<const char *>(&k_src_shader_prog_cnt), sizeof(k_src_shader_prog_cnt));

    // Pack textures and shader programs into assets file.
    if (!pack_textures(assets_file_ofs, src_asset_file_path, src_assets_dir_len) || !pack_shader_progs(assets_file_ofs, src_asset_file_path, src_assets_dir_len))
    {
        assets_file_ofs.close();
        std::remove(assets_file_path.c_str());
        return 1;
    }

    assets_file_ofs.close();

    return 0;
}
