#include <castle/c_assets.h>

#include <fstream>
#include <castle_common/cc_assets.h>
#include <castle_common/cc_misc.h>

c_asset_group::~c_asset_group()
{
    for (int i = 0; i < m_shader_prog_cnt; ++i)
    {
        glDeleteProgram(m_shader_prog_gl_ids[i]);
    }

    glDeleteTextures(m_tex_cnt, m_tex_gl_ids.get());
}

void c_asset_group::load_from_ifs(std::ifstream &ifs, const int tex_cnt, const int shader_prog_cnt)
{
    //
    // Textures
    //
    m_tex_cnt = tex_cnt;

    // Generate OpenGL textures and store their IDs.
    m_tex_gl_ids = std::make_unique<u_gl_id[]>(tex_cnt);
    glGenTextures(tex_cnt, m_tex_gl_ids.get());

    // Allocate a pixel data buffer, to be used as working space to store the pixel data for each texture.
    const int px_data_buf_size = cc::k_tex_channel_cnt * cc::k_tex_size_limit.x * cc::k_tex_size_limit.y;
    const auto px_data_buf = std::make_unique<unsigned char[]>(px_data_buf_size);

    // Read the sizes and pixel data of textures and finish setting them up.
    m_tex_sizes = std::make_unique<cc::s_vec_2d_int[]>(tex_cnt);

    for (int i = 0; i < tex_cnt; ++i)
    {
        ifs.read(reinterpret_cast<char *>(&m_tex_sizes[i]), sizeof(cc::s_vec_2d_int));

        ifs.read(reinterpret_cast<char *>(px_data_buf.get()), cc::k_tex_channel_cnt * m_tex_sizes[i].x * m_tex_sizes[i].y);

        glBindTexture(GL_TEXTURE_2D, m_tex_gl_ids[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_tex_sizes[i].x, m_tex_sizes[i].y, 0, GL_RGBA, GL_UNSIGNED_BYTE, px_data_buf.get());
    }

    //
    // Shader Programs
    //
    m_shader_prog_cnt = shader_prog_cnt;
    m_shader_prog_gl_ids = std::make_unique<u_gl_id[]>(shader_prog_cnt);

    for (int i = 0; i < shader_prog_cnt; ++i)
    {
        // Create the shaders using the sources in the file.
        const u_gl_id shader_gl_ids[2] = {
            glCreateShader(GL_VERTEX_SHADER),
            glCreateShader(GL_FRAGMENT_SHADER)
        };

        for (int j = 0; j < CC_STATIC_ARRAY_LEN(shader_gl_ids); ++j)
        {
            int src_size;
            ifs.read(reinterpret_cast<char *>(&src_size), sizeof(src_size));

            const auto src = std::make_unique<char[]>(src_size);
            ifs.read(src.get(), src_size);

            const char *const src_ptr = src.get();
            glShaderSource(shader_gl_ids[j], 1, &src_ptr, nullptr);

            glCompileShader(shader_gl_ids[j]);

            // Check for a shader compilation error.
            GLint gl_success;
            glGetShaderiv(shader_gl_ids[j], GL_COMPILE_STATUS, &gl_success);

            if (!gl_success)
            {
                GLint gl_info_log_len;
                glGetShaderiv(shader_gl_ids[j], GL_INFO_LOG_LENGTH, &gl_info_log_len);

                std::cout << "ERROR: OpenGL shader compilation failed!" << std::endl;

                if (gl_info_log_len > 0)
                {
                    const auto info_log = std::make_unique<char[]>(gl_info_log_len + 1);
                    glGetShaderInfoLog(shader_gl_ids[j], gl_info_log_len + 1, nullptr, info_log.get());

                    std::cout << "\n\n" << info_log << std::endl;
                }
            }
        }

        // Create the shader program using the shaders.
        m_shader_prog_gl_ids[i] = glCreateProgram();

        for (int j = 0; j < CC_STATIC_ARRAY_LEN(shader_gl_ids); ++j)
        {
            glAttachShader(m_shader_prog_gl_ids[i], shader_gl_ids[j]);
        }

        glLinkProgram(m_shader_prog_gl_ids[i]);

        // Delete the shaders as they're no longer needed.
        for (int j = CC_STATIC_ARRAY_LEN(shader_gl_ids) - 1; j >= 0; --j)
        {
            glDeleteShader(shader_gl_ids[j]);
        }
    }
}

bool c_assets::load_core_group()
{
    // Open the asset file.
    std::ifstream ifs(k_core_assets_file_name, std::ios::binary);

    if (!ifs.is_open())
    {
        std::cout << "ERROR: Failed to open \"" << k_core_assets_file_name << "\"!" << std::endl;
        return false;
    }

    // Read the file header and get asset counts.
    int tex_cnt;
    ifs.read(reinterpret_cast<char *>(&tex_cnt), sizeof(tex_cnt));

    int shader_prog_cnt;
    ifs.read(reinterpret_cast<char *>(&shader_prog_cnt), sizeof(shader_prog_cnt));

    // Load asset data.
    m_groups[0].load_from_ifs(ifs, tex_cnt, shader_prog_cnt);

    ifs.close();

    return true;
}
