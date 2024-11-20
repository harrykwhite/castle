#include <castle/c_assets.h>

#include <iostream>
#include <fstream>
#include <castle_common/cc_misc.h>

c_assets::~c_assets()
{
    for (int i = 0; i < k_asset_group_cnt; ++i)
    {
        if (m_group_activity.test(i))
        {
            dispose_group(i);
        }
    }
}

bool c_assets::load_core_group()
{
    assert(!m_group_activity.test(0));
    bool err = false;
    m_groups[0] = create_core_asset_group(err);
    m_group_activity.set(0);
    return !err;
}

void c_assets::load_and_dispose_mod_groups(const s_mods_state &mods_state)
{
    for (int i = 0; i < k_mod_limit; ++i)
    {
        const bool group_active = m_group_activity.test(1 + i);
        const bool mod_active = mods_state.mod_activity.test(i);

        if (group_active == mod_active)
        {
            continue;
        }

        if (mod_active)
        {
            // TODO: Load the mod group.
        }
        else
        {
            dispose_group(1 + i);
        }
    }
}

void c_assets::dispose_group(const int index)
{
    assert(index >= 0 && index < k_asset_group_cnt);
    assert(m_group_activity.test(index));

    glDeleteTextures(m_groups[index].font_cnt, m_groups[index].buf_font_tex_gl_ids);

    for (int i = 0; i < m_groups[index].shader_prog_cnt; ++i)
    {
        glDeleteProgram(m_groups[index].buf_shader_prog_gl_ids[i]);
    }

    glDeleteTextures(m_groups[index].tex_cnt, m_groups[index].buf_tex_gl_ids);

    delete[] m_groups[index].buf;

    m_groups[index] = {};

    m_group_activity.reset(index);
}

bool c_assets::load_mod_group(const int mod_index, std::ifstream &ifs, const int tex_cnt, const int shader_prog_cnt, const int font_cnt)
{
    assert(mod_index >= 0 && mod_index < k_mod_limit);
    assert(!m_group_activity.test(1 + mod_index));
    bool err = false;
    m_groups[1 + mod_index] = create_asset_group(ifs, tex_cnt, shader_prog_cnt, font_cnt);
    m_group_activity.set(1 + mod_index);
    return err;
}

s_asset_group create_asset_group(std::ifstream &ifs, const int tex_cnt, const int shader_prog_cnt, const int font_cnt)
{
    const int buf_tex_gl_ids_offs = 0;
    const int buf_tex_sizes_offs = sizeof(u_gl_id) * tex_cnt;
    const int buf_shader_prog_gl_ids_offs = buf_tex_sizes_offs + (sizeof(cc::s_vec_2d_i) * tex_cnt);
    const int buf_font_tex_gl_ids_offs = buf_shader_prog_gl_ids_offs + (sizeof(u_gl_id) * shader_prog_cnt);
    const int buf_font_datas_offs = buf_font_tex_gl_ids_offs + (sizeof(u_gl_id) * font_cnt);

    const int buf_size = buf_font_datas_offs + (sizeof(cc::s_font_data) * font_cnt);
    auto const buf = new cc::u_byte[buf_size];

    //
    // Textures
    //

    // Generate textures and store their IDs.
    const u_gl_id *const tex_gl_ids = [tex_cnt, buf]()
    {
        auto const tex_gl_ids = reinterpret_cast<u_gl_id *>(buf);
        glGenTextures(tex_cnt, tex_gl_ids);
        return tex_gl_ids;
    }();

    // Read the sizes and pixel data of textures and finish setting them up.
    const cc::s_vec_2d_i *const tex_sizes = [&ifs, tex_cnt, buf_tex_sizes_offs, buf, tex_gl_ids]()
    {
        auto const tex_sizes = reinterpret_cast<cc::s_vec_2d_i *>(buf + buf_tex_sizes_offs);

        // This buffer is working space for temporarily storing the pixel data of each texture.
        const int px_data_buf_size = cc::k_tex_channel_cnt * cc::k_tex_size_limit.x * cc::k_tex_size_limit.y;
        const auto px_data_buf = std::make_unique<unsigned char[]>(px_data_buf_size);

        for (int i = 0; i < tex_cnt; ++i)
        {
            ifs.read(reinterpret_cast<char *>(&tex_sizes[i]), sizeof(cc::s_vec_2d_i));

            ifs.read(reinterpret_cast<char *>(px_data_buf.get()), cc::k_tex_channel_cnt * tex_sizes[i].x * tex_sizes[i].y);

            glBindTexture(GL_TEXTURE_2D, tex_gl_ids[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_sizes[i].x, tex_sizes[i].y, 0, GL_RGBA, GL_UNSIGNED_BYTE, px_data_buf.get());
        }

        return tex_sizes;
    }();

    //
    // Shader Programs
    //
    const u_gl_id *const shader_prog_gl_ids = [&ifs, shader_prog_cnt, buf_shader_prog_gl_ids_offs, buf]()
    {
        auto const shader_prog_gl_ids = reinterpret_cast<u_gl_id *>(buf + buf_shader_prog_gl_ids_offs);

        for (int i = 0; i < shader_prog_cnt; ++i)
        {
            // Create the shaders using the sources in the file.
            const u_gl_id shader_gl_ids[2] = {
                glCreateShader(GL_VERTEX_SHADER),
                glCreateShader(GL_FRAGMENT_SHADER)
            };

            for (int j = 0; j < CC_STATIC_ARRAY_LEN(shader_gl_ids); ++j)
            {
                const auto src_size = cc::read_from_ifs<int>(ifs);

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
            shader_prog_gl_ids[i] = glCreateProgram();

            for (int j = 0; j < CC_STATIC_ARRAY_LEN(shader_gl_ids); ++j)
            {
                glAttachShader(shader_prog_gl_ids[i], shader_gl_ids[j]);
            }

            glLinkProgram(shader_prog_gl_ids[i]);

            // Delete the shaders as they're no longer needed.
            for (int j = CC_STATIC_ARRAY_LEN(shader_gl_ids) - 1; j >= 0; --j)
            {
                glDeleteShader(shader_gl_ids[j]);
            }
        }

        return shader_prog_gl_ids;
    }();

    //
    // Fonts
    //
    const u_gl_id *const font_tex_gl_ids = [&ifs, font_cnt, buf_font_tex_gl_ids_offs, buf]()
    {
        auto const font_tex_gl_ids = reinterpret_cast<u_gl_id *>(buf + buf_font_tex_gl_ids_offs);
        glGenTextures(font_cnt, font_tex_gl_ids);
        return font_tex_gl_ids;
    }();

    const cc::s_font_data *const font_datas = [&ifs, font_cnt, buf_font_datas_offs, buf, font_tex_gl_ids]()
    {
        auto const font_datas = reinterpret_cast<cc::s_font_data *>(buf + buf_font_datas_offs);

        const int px_data_buf_size = cc::k_font_tex_channel_cnt * cc::k_tex_size_limit.x * cc::k_tex_size_limit.y;
        const auto px_data_buf = std::make_unique<unsigned char[]>(px_data_buf_size);

        for (int i = 0; i < font_cnt; ++i)
        {
            ifs.read(reinterpret_cast<char *>(&font_datas[i]), sizeof(cc::s_font_data));

            ifs.read(reinterpret_cast<char *>(px_data_buf.get()), cc::k_font_tex_channel_cnt * font_datas[i].tex_size.x * font_datas[i].tex_size.y);

            glBindTexture(GL_TEXTURE_2D, font_tex_gl_ids[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, font_datas[i].tex_size.x, font_datas[i].tex_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, px_data_buf.get());
        }

        return font_datas;
    }();

    return {
        buf,
        tex_gl_ids,
        tex_sizes,
        shader_prog_gl_ids,
        font_tex_gl_ids,
        font_datas,
        tex_cnt,
        shader_prog_cnt,
        font_cnt
    };
}

s_asset_group create_core_asset_group(bool &err)
{
    // Open the asset file.
    std::ifstream ifs(k_core_assets_file_name, std::ios::binary);

    if (!ifs.is_open())
    {
        // TODO: Handle error properly.
        err = true;
        return {};
    }

    // Read the file header and get asset counts.
    const auto tex_cnt = cc::read_from_ifs<int>(ifs);
    const auto shader_prog_cnt = cc::read_from_ifs<int>(ifs);
    const auto font_cnt = cc::read_from_ifs<int>(ifs);

    // Make the asset group from the rest of the file.
    return create_asset_group(ifs, tex_cnt, shader_prog_cnt, font_cnt);
}
