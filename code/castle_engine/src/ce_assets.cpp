#include <castle_engine/ce_assets.h>

#include <string>
#include <stdexcept>
#include <cstdlib>
#include <castle_engine/ce_game.h>
#include <castle_common/cc_misc.h>
#include <castle_common/cc_assets.h>
#include <castle_common/cc_debugging.h>
#include <castle_common/cc_io.h>

namespace ce
{

bool init_assets_with_file(s_assets *const assets, std::FILE *const fs, char *const err_msg_buf)
{
    std::memset(assets, 0, sizeof(*assets));

    // Read the file header.
    assets->tex_cnt = cc::read_next_item_from_file<int>(fs);
    assets->shader_prog_cnt = cc::read_next_item_from_file<int>(fs);

    // Determine the buffer size and offsets.
    assets->buf_size = 0;

    assets->buf_tex_gl_ids_offs = assets->buf_size;
    assets->buf_size += sizeof(u_gl_id) * assets->tex_cnt;

    assets->buf_tex_sizes_offs = assets->buf_size;
    assets->buf_size += sizeof(cc::s_vec_2d_int) * assets->tex_cnt;

    assets->buf_shader_prog_gl_ids_offs = assets->buf_size;
    assets->buf_size += sizeof(u_gl_id) * assets->shader_prog_cnt;

    // Allocate the assets buffer.
    assets->buf = std::malloc(assets->buf_size);

    if (!assets->buf)
    {
        std::snprintf(err_msg_buf, k_err_msg_buf_size, "Failed to allocate %d bytes for an assets buffer!", assets->buf_size);
        return false;
    }

    //
    // Textures
    //
    {
        // Generate OpenGL textures and store them in the buffer.
        glGenTextures(assets->tex_cnt, get_tex_gl_ids(assets));

        // Allocate a pixel data buffer, to be used as working space to store the pixel data for each texture.
        const int px_data_buf_size = cc::k_tex_channel_cnt * cc::k_tex_size_limit.x * cc::k_tex_size_limit.y;
        auto const px_data_buf = reinterpret_cast<unsigned char *>(std::malloc(px_data_buf_size));

        if (!px_data_buf)
        {
            std::snprintf(err_msg_buf, k_err_msg_buf_size, "Failed to allocate %d bytes for a texture pixel data buffer!", px_data_buf_size);
            return false;
        }

        // Read the sizes and pixel data of textures and finish setting them up.
        for (int i = 0; i < assets->tex_cnt; ++i)
        {
            cc::s_vec_2d_int &tex_size = get_tex_sizes(assets)[i];
            tex_size = cc::read_next_item_from_file<cc::s_vec_2d_int>(fs);

            std::fread(px_data_buf, 1, cc::k_tex_channel_cnt * tex_size.x * tex_size.y, fs);

            glBindTexture(GL_TEXTURE_2D, get_tex_gl_ids(assets)[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_size.x, tex_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, px_data_buf);
        }

        std::free(px_data_buf);
    }

    //
    // Shader Programs
    //
    for (int i = 0; i < assets->shader_prog_cnt; ++i)
    {
        // Create the shaders using the sources in the file.
        const u_gl_id shader_gl_ids[2] = {
            glCreateShader(GL_VERTEX_SHADER),
            glCreateShader(GL_FRAGMENT_SHADER)
        };

        for (int j = 0; j < CC_STATIC_ARRAY_LEN(shader_gl_ids); ++j)
        {
            const auto src_size = cc::read_next_item_from_file<int>(fs);

            auto const src = reinterpret_cast<char *>(std::malloc(src_size));

            if (!src)
            {
                std::snprintf(err_msg_buf, k_err_msg_buf_size, "Failed to allocate %d bytes to store a shader source!", src_size);

                for (int k = CC_STATIC_ARRAY_LEN(shader_gl_ids) - 1; k >= 0; --k)
                {
                    glDeleteShader(shader_gl_ids[k]);
                }

                return false;
            }

            std::fread(src, 1, src_size, fs);
            glShaderSource(shader_gl_ids[j], 1, &src, nullptr);

            std::free(src);

            glCompileShader(shader_gl_ids[j]);
        }

        // Create the shader program using the shaders.
        ce::u_gl_id &prog_gl_id = get_shader_prog_gl_ids(assets)[i];

        prog_gl_id = glCreateProgram();

        for (int j = 0; j < CC_STATIC_ARRAY_LEN(shader_gl_ids); ++j)
        {
            glAttachShader(prog_gl_id, shader_gl_ids[j]);
        }

        glLinkProgram(prog_gl_id);

        // Delete the shaders as they're no longer needed.
        for (int j = CC_STATIC_ARRAY_LEN(shader_gl_ids) - 1; j >= 0; --j)
        {
            glDeleteShader(shader_gl_ids[j]);
        }

        ++assets->shader_prog_init_cnt;
    }

    return true;
}

void clean_assets(s_assets *const assets)
{
    for (int i = 0; i < assets->shader_prog_init_cnt; ++i)
    {
        glDeleteProgram(get_shader_prog_gl_ids(assets)[i]);
    }

    if (assets->tex_init)
    {
        glDeleteTextures(assets->tex_cnt, get_tex_gl_ids(assets));
    }

    if (assets->buf)
    {
        std::free(assets->buf);
    }

    std::memset(assets, 0, sizeof(*assets));
}

}
