#include <castle_common/cc_assets.h>
#include <castle_common/cc_debugging.h>
#include <castle_common/cc_math.h>
#include <castle_common/cc_misc.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stb_image.h>

#if 0
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

struct s_src_shader_prog_info
{
    const char *vs_file_path_end;
    const char *fs_file_path_end;
};

constexpr int k_src_asset_file_path_buf_size = 128;

const char *const k_src_tex_file_path_ends[] = {
    "/textures/characters/player.png",
    "/textures/tiles/dirt.png",
    "/textures/tiles/stone.png"
};

const s_src_shader_prog_info k_src_shader_prog_infos[] = {
    {"/shaders/screen.vert", "/shaders/screen.frag"},
    {"/shaders/quad.vert", "/shaders/quad.frag"},
    {"/shaders/line.vert", "/shaders/line.frag"}
};

constexpr int k_src_tex_cnt = CC_STATIC_ARRAY_LEN(k_src_tex_file_path_ends);
constexpr int k_src_shader_prog_cnt = CC_STATIC_ARRAY_LEN(k_src_shader_prog_infos);

static bool pack_textures(std::FILE *const assets_file_fs, char *const src_asset_file_path_buf, const int src_assets_dir_len)
{
    for (int i = 0; i < k_src_tex_cnt; i++)
    {
        // Copy the texture file path end to the asset file path buffer.
        const int src_file_path_len = cc::copy_str_and_get_len(k_src_tex_file_path_ends[i], src_asset_file_path_buf + src_assets_dir_len, k_src_asset_file_path_buf_size - 1 - src_assets_dir_len);

        if (src_file_path_len == -1)
        {
            cc::log_error("The source texture file path end of \"%s\" is too long!", k_src_tex_file_path_ends[i]);
            return false;
        }

        // Load and write the texture.
        cc::s_vec_2d_int tex_size;
        stbi_uc *const px_data = stbi_load(src_asset_file_path_buf, &tex_size.x, &tex_size.y, nullptr, cc::k_tex_channel_cnt);

        if (!px_data)
        {
            cc::log_error("\"stbi_load\" function failed: \"%s\"", stbi_failure_reason());
            return false;
        }

        if (tex_size.x > cc::k_tex_size_limit.x || tex_size.y > cc::k_tex_size_limit.y)
        {
            cc::log_error("The width or height of the source texture with file path \"%s\" exceeds the limit of %dx%d!", src_asset_file_path_buf, cc::k_tex_size_limit.x, cc::k_tex_size_limit.y);
            stbi_image_free(px_data);
            return false;
        }

        std::fwrite(&tex_size, sizeof(tex_size), 1, assets_file_fs);
        std::fwrite(px_data, 1, tex_size.x * tex_size.y * cc::k_tex_channel_cnt, assets_file_fs);

        stbi_image_free(px_data);

        cc::log("Successfully packed texture with file path \"%s\".", src_asset_file_path_buf);
    }

    return true;
}

static bool pack_shader_progs(std::FILE *const assets_file_fs, char *const src_asset_file_path_buf, const int src_assets_dir_len)
{
    for (int i = 0; i < k_src_shader_prog_cnt; i++)
    {
        // Pack vertex shader, then fragment shader.
        for (int j = 0; j < 2; j++)
        {
            // Copy the shader file path end to the asset file path buffer.
            const char *const src_shader_file_path_end = j == 0 ? k_src_shader_prog_infos[i].vs_file_path_end : k_src_shader_prog_infos[i].fs_file_path_end;
            const int src_shader_file_path_len = cc::copy_str_and_get_len(src_shader_file_path_end, src_asset_file_path_buf + src_assets_dir_len, k_src_asset_file_path_buf_size - 1 - src_assets_dir_len);

            if (src_shader_file_path_len == -1)
            {
                cc::log_error("The source shader file path end \"%s\" is too long!", src_shader_file_path_end);
                return false;
            }

            // Try to open the shader source file.
            std::FILE *const shader_fs = std::fopen(src_asset_file_path_buf, "rb");

            if (!shader_fs)
            {
                cc::log_error("Failed to open %s!", src_shader_file_path_end);
                return false;
            }

            // Get the shader source size, and write it to the assets file.
            std::fseek(shader_fs, 0, SEEK_END);
            const int shader_src_size = std::ftell(shader_fs) + 1;

            std::fwrite(&shader_src_size, sizeof(shader_src_size), 1, assets_file_fs);

            // Store the shader source contents in a buffer, and write it to the assets file.
            auto const shader_src = reinterpret_cast<char *>(std::malloc(shader_src_size));

            if (!shader_src)
            {
                cc::log_error("Failed to allocate %d bytes to store a shader source!", shader_src_size);
                return false;
            }

            std::fseek(shader_fs, 0, SEEK_SET);
            std::fread(shader_src, 1, shader_src_size - 1, shader_fs);
            shader_src[shader_src_size - 1] = '\0';

            std::fwrite(shader_src, 1, shader_src_size, assets_file_fs);

            std::fclose(shader_fs);

            std::free(shader_src);

            cc::log("Successfully packed shader with file path \"%s\".", src_asset_file_path_buf);
        }
    }

    return true;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cc::log_error("An assets source directory and an output assets file path must both be provided as command-line arguments!");
        return 1;
    }

    // Allocate the source asset file path buffer. Asset file paths are to be written into here, following the source assets directory. There's no need to clear this, as the cases where this would contain a non-null-terminated string are all handled.
    char src_asset_file_path_buf[k_src_asset_file_path_buf_size];

    // Write the source assets directory to asset file path buffer.
    const int src_assets_dir_len = cc::copy_str_and_get_len(argv[1], src_asset_file_path_buf, sizeof(src_asset_file_path_buf) - 1);

    if (src_assets_dir_len == -1)
    {
        cc::log_error("The source assets directory is too long!");
        return 1;
    }

    // Create or truncate the assets file using the provided path.
    const char *const assets_file_path = argv[2];

    std::FILE *const assets_file_fs = std::fopen(assets_file_path, "wb");

    if (!assets_file_fs)
    {
        cc::log_error("Failed to open assets file with path \"%s\"!", assets_file_path);
        return 1;
    }

    // Write the asset counts to the assets file header.
    std::fwrite(&k_src_tex_cnt, sizeof(k_src_tex_cnt), 1, assets_file_fs);
    std::fwrite(&k_src_shader_prog_cnt, sizeof(k_src_shader_prog_cnt), 1, assets_file_fs);

    // Pack the assets.
    if (!pack_textures(assets_file_fs, src_asset_file_path_buf, src_assets_dir_len) || !pack_shader_progs(assets_file_fs, src_asset_file_path_buf, src_assets_dir_len))
    {
        fclose(assets_file_fs);
        remove(assets_file_path);

        return 1;
    }

    std::fclose(assets_file_fs);

    return 0;
}
