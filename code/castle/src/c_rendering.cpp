#include <castle/c_rendering.h>

static inline cc::s_matrix_4x4 make_camera_view_matrix(const s_camera &cam, const cc::s_vec_2d_i window_size)
{
    auto mat = cc::s_matrix_4x4::identity();
    mat[0][0] = cam.scale;
    mat[1][1] = cam.scale;
    mat[3][0] = (-cam.pos.x * cam.scale) + (window_size.x / 2.0f);
    mat[3][1] = (-cam.pos.y * cam.scale) + (window_size.y / 2.0f);
    return mat;
}

c_sprite_batch::c_sprite_batch(const int slot_cnt) : m_slot_cnt(slot_cnt), m_slot_activity(slot_cnt / 8), m_slot_tex_units(std::make_unique<int[]>(slot_cnt))
{
    assert(slot_cnt % 8 == 0);

    // Generate vertex array.
    glGenVertexArrays(1, &m_vert_array_gl_id);
    glBindVertexArray(m_vert_array_gl_id);

    // Generate vertex buffer.
    glGenBuffers(1, &m_vert_buf_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_vert_buf_gl_id);

    {
        const int verts_len = k_sprite_quad_shader_prog_vert_cnt * 4 * slot_cnt;
        const auto verts = std::make_unique<const float[]>(verts_len);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts[0]) * verts_len, verts.get(), GL_DYNAMIC_DRAW);
    }

    // Generate element buffer.
    glGenBuffers(1, &m_elem_buf_gl_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elem_buf_gl_id);

    {
        // TODO: Have it so that the indices are only set up once; all batches use a copy of this same data, so recalculation isn't needed.
        const int indices_len = 6 * slot_cnt;
        const auto indices = std::make_unique<unsigned short[]>(indices_len);

        for (int i = 0; i < slot_cnt; i++)
        {
            indices[(i * 6) + 0] = (i * 4) + 0;
            indices[(i * 6) + 1] = (i * 4) + 1;
            indices[(i * 6) + 2] = (i * 4) + 2;
            indices[(i * 6) + 3] = (i * 4) + 2;
            indices[(i * 6) + 4] = (i * 4) + 3;
            indices[(i * 6) + 5] = (i * 4) + 0;
        }

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices_len, indices.get(), GL_STATIC_DRAW);
    }

    // Set vertex attribute pointers.
    const int verts_stride = sizeof(float) * k_sprite_quad_shader_prog_vert_cnt;

    glVertexAttribPointer(0, 2, GL_FLOAT, false, verts_stride, reinterpret_cast<void *>(sizeof(float) * 0));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, false, verts_stride, reinterpret_cast<void *>(sizeof(float) * 2));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, false, verts_stride, reinterpret_cast<void *>(sizeof(float) * 4));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 1, GL_FLOAT, false, verts_stride, reinterpret_cast<void *>(sizeof(float) * 6));
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, 1, GL_FLOAT, false, verts_stride, reinterpret_cast<void *>(sizeof(float) * 7));
    glEnableVertexAttribArray(4);

    glVertexAttribPointer(5, 2, GL_FLOAT, false, verts_stride, reinterpret_cast<void *>(sizeof(float) * 8));
    glEnableVertexAttribArray(5);

    glVertexAttribPointer(6, 1, GL_FLOAT, false, verts_stride, reinterpret_cast<void *>(sizeof(float) * 10));
    glEnableVertexAttribArray(6);

    glBindVertexArray(0);
}

c_sprite_batch::~c_sprite_batch()
{
    glDeleteBuffers(1, &m_elem_buf_gl_id);
    glDeleteBuffers(1, &m_vert_buf_gl_id);
    glDeleteVertexArrays(1, &m_vert_array_gl_id);
}

c_sprite_batch::c_sprite_batch(c_sprite_batch &&other)
    : m_vert_array_gl_id(other.m_vert_array_gl_id),
    m_vert_buf_gl_id(other.m_vert_buf_gl_id),
    m_elem_buf_gl_id(other.m_elem_buf_gl_id),
    m_slot_cnt(other.m_slot_cnt),
    m_slot_activity(std::move(other.m_slot_activity)),
    m_slot_tex_units(std::move(other.m_slot_tex_units))
{
    std::copy(std::begin(other.m_tex_unit_tex_ids), std::end(other.m_tex_unit_tex_ids), m_tex_unit_tex_ids);
    std::copy(std::begin(other.m_tex_unit_ref_cnts), std::end(other.m_tex_unit_ref_cnts), m_tex_unit_ref_cnts);

    other.m_vert_array_gl_id = 0;
    other.m_vert_buf_gl_id = 0;
    other.m_elem_buf_gl_id = 0;
}

c_sprite_batch &c_sprite_batch::operator=(c_sprite_batch &&other)
{
    if (this != &other)
    {
        m_vert_array_gl_id = other.m_vert_array_gl_id;
        m_vert_buf_gl_id = other.m_vert_buf_gl_id;
        m_elem_buf_gl_id = other.m_elem_buf_gl_id;
        m_slot_cnt = other.m_slot_cnt;
        m_slot_activity = std::move(other.m_slot_activity);
        m_slot_tex_units = std::move(other.m_slot_tex_units);

        std::copy(std::begin(other.m_tex_unit_tex_ids), std::end(other.m_tex_unit_tex_ids), m_tex_unit_tex_ids);
        std::copy(std::begin(other.m_tex_unit_ref_cnts), std::end(other.m_tex_unit_ref_cnts), m_tex_unit_ref_cnts);

        other.m_vert_array_gl_id = 0;
        other.m_vert_buf_gl_id = 0;
        other.m_elem_buf_gl_id = 0;
    }

    return *this;
}

void c_sprite_batch::draw(const c_assets &assets, const cc::s_vec_2d_i window_size, const s_camera *const cam) const
{
    const u_gl_id sprite_quad_prog_gl_id = assets.get_shader_prog_gl_id(s_asset_id::make_core_shader_prog_id(ec_core_shader_prog::sprite_quad));

    glUseProgram(sprite_quad_prog_gl_id);

    // Set up the projection matrix.
    const auto proj_mat = cc::s_matrix_4x4::ortho(0.0f, window_size.x, window_size.y, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(sprite_quad_prog_gl_id, "u_proj"), 1, false, reinterpret_cast<const float *>(proj_mat.elems));

    // Set up the view matrix.
    const auto view_mat = cam ? make_camera_view_matrix(*cam, window_size) : cc::s_matrix_4x4::identity();
    glUniformMatrix4fv(glGetUniformLocation(sprite_quad_prog_gl_id, "u_view"), 1, false, reinterpret_cast<const float *>(view_mat.elems));

    // Set up texture units.
    int tex_units[k_tex_unit_limit];
    std::iota(tex_units, tex_units + k_tex_unit_limit, 0);

    for (int i = 0; i < k_tex_unit_limit; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_tex_unit_ref_cnts[i] > 0 ? assets.get_tex_gl_id(m_tex_unit_tex_ids[i]) : 0);
    }

    glUniform1iv(glGetUniformLocation(sprite_quad_prog_gl_id, "u_textures"), k_tex_unit_limit, tex_units);

    // Draw the batch.
    glBindVertexArray(m_vert_array_gl_id);
    glDrawElements(GL_TRIANGLES, 6 * m_slot_cnt, GL_UNSIGNED_SHORT, nullptr);
}

int c_sprite_batch::take_any_slot(const s_asset_id tex_id)
{
    // If there are no inactive slots, don't go any further.
    if (m_slot_activity.is_full())
    {
        return -1;
    }

    // Try to find a texture unit to use, and don't go any further if not found.
    const int tex_unit = find_tex_unit_to_use(tex_id);

    if (tex_unit == -1)
    {
        return -1;
    }

    // Use the first inactive slot.
    const int slot_index = m_slot_activity.get_first_inactive_bit_index();

    // Mark the slot as active.
    m_slot_activity.activate_bit(slot_index);

    // Update texture unit information.
    ++m_tex_unit_ref_cnts[tex_unit];
    m_tex_unit_tex_ids[tex_unit] = tex_id;
    m_slot_tex_units[slot_index] = tex_unit;

    return slot_index;
}

void c_sprite_batch::release_slot(const int slot_index)
{
    // Mark the slot as inactive.
    m_slot_activity.deactivate_bit(slot_index);

    // Update texture unit information.
    const int tex_unit = m_slot_tex_units[slot_index];
    --m_tex_unit_ref_cnts[tex_unit];

    // Clear the slot render data.
    clear_slot(slot_index);
}

void c_sprite_batch::write_to_slot(const int slot_index, const c_assets &assets, const cc::s_vec_2d pos, const cc::s_rect &src_rect, const cc::s_vec_2d origin, const float rot, const cc::s_vec_2d scale, const float alpha) const
{
    const int tex_unit = m_slot_tex_units[slot_index];
    const cc::s_vec_2d_i tex_size = assets.get_tex_size(m_tex_unit_tex_ids[tex_unit]);

    const float verts[k_sprite_quad_shader_prog_vert_cnt * 4] = {
        (0.0f - origin.x) * scale.x,
        (0.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        static_cast<float>(src_rect.width),
        static_cast<float>(src_rect.height),
        rot,
        static_cast<float>(tex_unit),
        static_cast<float>(src_rect.x) / tex_size.x,
        static_cast<float>(src_rect.y) / tex_size.y,
        alpha,

        (1.0f - origin.x) * scale.x,
        (0.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        static_cast<float>(src_rect.width),
        static_cast<float>(src_rect.height),
        rot,
        static_cast<float>(tex_unit),
        static_cast<float>(src_rect.x + src_rect.width) / tex_size.x,
        static_cast<float>(src_rect.y) / tex_size.y,
        alpha,

        (1.0f - origin.x) * scale.x,
        (1.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        static_cast<float>(src_rect.width),
        static_cast<float>(src_rect.height),
        rot,
        static_cast<float>(tex_unit),
        static_cast<float>(src_rect.x + src_rect.width) / tex_size.x,
        static_cast<float>(src_rect.y + src_rect.height) / tex_size.y,
        alpha,

        (0.0f - origin.x) * scale.x,
        (1.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        static_cast<float>(src_rect.width),
        static_cast<float>(src_rect.height),
        rot,
        static_cast<float>(tex_unit),
        static_cast<float>(src_rect.x) / tex_size.x,
        static_cast<float>(src_rect.y + src_rect.height) / tex_size.y,
        alpha
    };

    glBindVertexArray(m_vert_array_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_vert_buf_gl_id);
    glBufferSubData(GL_ARRAY_BUFFER, slot_index * sizeof(verts), sizeof(verts), verts);
}

void c_sprite_batch::clear_slot(const int slot_index) const
{
    glBindVertexArray(m_vert_array_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_vert_buf_gl_id);

    const float verts[k_sprite_quad_shader_prog_vert_cnt * 4] = {};
    glBufferSubData(GL_ARRAY_BUFFER, slot_index * sizeof(verts), sizeof(verts), verts);
}

int c_sprite_batch::find_tex_unit_to_use(const s_asset_id tex_id) const
{
    int free_tex_unit = -1;

    for (int i = 0; i < k_tex_unit_limit; ++i)
    {
        if (!m_tex_unit_ref_cnts[i])
        {
            if (free_tex_unit == -1)
            {
                free_tex_unit = i;
            }

            continue;
        }

        if (m_tex_unit_tex_ids[i] == tex_id)
        {
            return i;
        }
    }

    return free_tex_unit;
}

c_char_batch::c_char_batch(const int slot_cnt, const s_asset_id font_id) : m_slot_cnt(slot_cnt), m_font_id(font_id)
{
    assert(slot_cnt > 0);

    // Generate vertex array.
    glGenVertexArrays(1, &m_vert_array_gl_id);
    glBindVertexArray(m_vert_array_gl_id);

    // Generate vertex buffer.
    glGenBuffers(1, &m_vert_buf_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_vert_buf_gl_id);

    {
        const int verts_len = k_char_quad_shader_prog_vert_cnt * 4 * slot_cnt;
        const auto verts = std::make_unique<const float[]>(verts_len);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts[0]) * verts_len, verts.get(), GL_DYNAMIC_DRAW);
    }

    // Generate element buffer.
    glGenBuffers(1, &m_elem_buf_gl_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elem_buf_gl_id);

    {
        // TODO: Have it so that the indices are only set up once; all batches use a copy of this same data, so recalculation isn't needed.
        const int indices_len = 6 * slot_cnt;
        const auto indices = std::make_unique<unsigned short[]>(indices_len);

        for (int i = 0; i < slot_cnt; i++)
        {
            indices[(i * 6) + 0] = (i * 4) + 0;
            indices[(i * 6) + 1] = (i * 4) + 1;
            indices[(i * 6) + 2] = (i * 4) + 2;
            indices[(i * 6) + 3] = (i * 4) + 2;
            indices[(i * 6) + 4] = (i * 4) + 3;
            indices[(i * 6) + 5] = (i * 4) + 0;
        }

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices_len, indices.get(), GL_STATIC_DRAW);
    }

    // Set vertex attribute pointers.
    const int verts_stride = sizeof(float) * k_char_quad_shader_prog_vert_cnt;

    glVertexAttribPointer(0, 2, GL_FLOAT, false, verts_stride, reinterpret_cast<void *>(sizeof(float) * 0));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, false, verts_stride, reinterpret_cast<void *>(sizeof(float) * 2));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

c_char_batch::~c_char_batch()
{
    glDeleteBuffers(1, &m_elem_buf_gl_id);
    glDeleteBuffers(1, &m_vert_buf_gl_id);
    glDeleteVertexArrays(1, &m_vert_array_gl_id);
}

c_char_batch::c_char_batch(c_char_batch &&other)
    : m_pos(other.m_pos),
    m_rot(other.m_rot),
    m_blend(other.m_blend),
    m_vert_array_gl_id(other.m_vert_array_gl_id),
    m_vert_buf_gl_id(other.m_vert_buf_gl_id),
    m_elem_buf_gl_id(other.m_elem_buf_gl_id),
    m_active_slot_cnt(other.m_active_slot_cnt),
    m_slot_cnt(other.m_slot_cnt),
    m_font_id(other.m_font_id)
{
    other.m_vert_array_gl_id = 0;
    other.m_vert_buf_gl_id = 0;
    other.m_elem_buf_gl_id = 0;
}

c_char_batch &c_char_batch::operator=(c_char_batch &&other)
{
    if (this != &other)
    {
        m_pos = other.m_pos;
        m_rot = other.m_rot;
        m_blend = other.m_blend;
        m_vert_array_gl_id = other.m_vert_array_gl_id;
        m_vert_buf_gl_id = other.m_vert_buf_gl_id;
        m_elem_buf_gl_id = other.m_elem_buf_gl_id;
        m_active_slot_cnt = other.m_active_slot_cnt;
        m_slot_cnt = other.m_slot_cnt;
        m_font_id = other.m_font_id;

        other.m_vert_array_gl_id = 0;
        other.m_vert_buf_gl_id = 0;
        other.m_elem_buf_gl_id = 0;
    }

    return *this;
}

void c_char_batch::write(const std::string &text, const c_assets &assets, const ec_font_align_hor align_hor, const ec_font_align_ver align_ver)
{
    assert(text.length() > 0 && m_slot_cnt <= text.length());

    m_active_slot_cnt = text.length();

    const cc::s_font_data &font_data = assets.get_font_data(m_font_id);

    // Determine the positions of text characters based on font information, alongside the overall dimensions of the text to be used when applying alignment.
    std::vector<cc::s_vec_2d> char_draw_positions(m_slot_cnt);
    cc::s_vec_2d char_draw_pos_pen = {0};

    std::vector<int> text_line_widths(m_slot_cnt);
    int text_first_line_min_offs;
    bool text_first_line_min_offs_updated = false;
    int text_last_line_max_height;
    bool text_last_line_max_height_updated = false;
    int text_line_counter = 0;

    for (int i = 0; i < text.length(); i++)
    {
        if (text[i] == '\n')
        {
            text_line_widths[text_line_counter] = char_draw_pos_pen.x;

            if (!text_first_line_min_offs_updated)
            {
                // Set the first line minimum offset to the vertical offset of the space character.
                text_first_line_min_offs = font_data.chars.ver_offsets[0];
                text_first_line_min_offs_updated = true;
            }

            // Set the last line maximum height to the height of a space.
            text_last_line_max_height = font_data.chars.ver_offsets[0] + font_data.chars.src_rects[0].height;

            text_last_line_max_height_updated = false;

            text_line_counter++;

            // Move the pen to a new line.
            char_draw_pos_pen.x = 0.0f;
            char_draw_pos_pen.y += font_data.line_height;

            continue;
        }

        const int text_char_index = text[i] - cc::k_font_char_range_begin;

        // If we are on the first line, update the first line minimum offset.
        if (text_line_counter == 0)
        {
            if (!text_first_line_min_offs_updated)
            {
                text_first_line_min_offs = font_data.chars.ver_offsets[text_char_index];
                text_first_line_min_offs_updated = true;
            }
            else
            {
                text_first_line_min_offs = std::min(font_data.chars.ver_offsets[text_char_index], text_first_line_min_offs);
            }
        }

        if (!text_last_line_max_height_updated)
        {
            text_last_line_max_height = font_data.chars.ver_offsets[text_char_index] + font_data.chars.src_rects[text_char_index].height;
            text_last_line_max_height_updated = true;
        }
        else
        {
            text_last_line_max_height = std::max(font_data.chars.ver_offsets[text_char_index] + font_data.chars.src_rects[text_char_index].height, text_last_line_max_height);
        }

        if (i > 0)
        {
            // Apply kerning based on the previous character.
            const int text_char_index_last = text[i - 1] - cc::k_font_char_range_begin;
            char_draw_pos_pen.x += font_data.chars.kernings[(text_char_index * cc::k_font_char_range_size) + text_char_index_last];
        }

        char_draw_positions[i].x = char_draw_pos_pen.x + font_data.chars.hor_offsets[text_char_index];
        char_draw_positions[i].y = char_draw_pos_pen.y + font_data.chars.ver_offsets[text_char_index];

        char_draw_pos_pen.x += font_data.chars.hor_advances[text_char_index];
    }

    text_line_widths[text_line_counter] = char_draw_pos_pen.x;
    text_line_counter = 0;

    const int text_height = text_first_line_min_offs + char_draw_pos_pen.y + text_last_line_max_height;

    // Clear the batch then write the character render data.
    glBindVertexArray(m_vert_array_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_vert_buf_gl_id);

    {
        const std::vector<float> batch_clear_verts((k_char_quad_shader_prog_vert_cnt * 4) * m_slot_cnt);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(batch_clear_verts), batch_clear_verts.data());
    }

    for (int i = 0; i < text.length(); i++)
    {
        if (text[i] == '\n')
        {
            text_line_counter++;
            continue;
        }

        if (text[i] == ' ')
        {
            continue;
        }

        const int char_index = text[i] - cc::k_font_char_range_begin;

        const cc::s_vec_2d char_draw_pos = {
            char_draw_positions[i].x - (text_line_widths[text_line_counter] * (static_cast<int>(align_hor) / 2.0f)),
            char_draw_positions[i].y - (text_height * (static_cast<int>(align_ver) / 2.0f))
        };

        const cc::s_rect_f char_tex_coords_rect = {
            static_cast<float>(font_data.chars.src_rects[char_index].x) / font_data.tex_size.x,
            static_cast<float>(font_data.chars.src_rects[char_index].y) / font_data.tex_size.y,
            static_cast<float>(font_data.chars.src_rects[char_index].width) / font_data.tex_size.x,
            static_cast<float>(font_data.chars.src_rects[char_index].height) / font_data.tex_size.y
        };

        const float verts[k_char_quad_shader_prog_vert_cnt * 4] = {
            char_draw_pos.x,
            char_draw_pos.y,
            char_tex_coords_rect.x,
            char_tex_coords_rect.y,

            char_draw_pos.x + font_data.chars.src_rects[char_index].width,
            char_draw_pos.y,
            char_tex_coords_rect.x + char_tex_coords_rect.width,
            char_tex_coords_rect.y,

            char_draw_pos.x + font_data.chars.src_rects[char_index].width,
            char_draw_pos.y + font_data.chars.src_rects[char_index].height,
            char_tex_coords_rect.x + char_tex_coords_rect.width,
            char_tex_coords_rect.y + char_tex_coords_rect.height,

            char_draw_pos.x,
            char_draw_pos.y + font_data.chars.src_rects[char_index].height,
            char_tex_coords_rect.x,
            char_tex_coords_rect.y + char_tex_coords_rect.height
        };

        glBufferSubData(GL_ARRAY_BUFFER, i * sizeof(verts), sizeof(verts), verts);
    }
}

void c_char_batch::draw(const c_assets &assets, const cc::s_vec_2d_i window_size) const
{
    const u_gl_id char_quad_prog_gl_id = assets.get_shader_prog_gl_id(s_asset_id::make_core_shader_prog_id(ec_core_shader_prog::char_quad));
    glUseProgram(char_quad_prog_gl_id);

    const auto proj_mat = cc::s_matrix_4x4::ortho(0.0f, window_size.x, window_size.y, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(char_quad_prog_gl_id, "u_proj"), 1, false, reinterpret_cast<const float *>(proj_mat.elems));

    glUniform2fv(glGetUniformLocation(char_quad_prog_gl_id, "u_pos"), 1, reinterpret_cast<const float *>(&m_pos));
    glUniform1f(glGetUniformLocation(char_quad_prog_gl_id, "u_rot"), m_rot);
    glUniform4fv(glGetUniformLocation(char_quad_prog_gl_id, "u_blend"), 1, reinterpret_cast<const float *>(&m_blend));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, assets.get_font_tex_gl_id(m_font_id));

    glBindVertexArray(m_vert_array_gl_id);
    glDrawElements(GL_TRIANGLES, 6 * m_active_slot_cnt, GL_UNSIGNED_SHORT, nullptr);
}

c_renderer::c_renderer(const std::vector<s_render_layer_init_info> &&layer_init_infos, const int sprite_batch_cam_layer_cnt) : m_layer_init_infos(layer_init_infos), m_sprite_batch_cam_layer_cnt(sprite_batch_cam_layer_cnt), m_layer_infos(std::make_unique<s_render_layer_info[]>(layer_init_infos.size()))
{
    // Initialise layer information.
    for (int i = 0; i < get_layer_cnt(); ++i)
    {
        m_layer_infos[i].begin_sprite_batch_index = i;
        m_layer_infos[i].sprite_batch_cnt = 1;
        m_layer_infos[i].begin_char_batch_index = i;
        m_layer_infos[i].char_batch_cnt = 1;
    }

    // Initialise batches.
    m_sprite_batches.reserve(get_layer_cnt());
    m_char_batches.reserve(get_layer_cnt());

    for (const s_render_layer_init_info &layer_init_info : m_layer_init_infos)
    {
        m_sprite_batches.emplace_back(layer_init_info.sprite_batch_default_slot_cnt);
        m_char_batches.emplace_back(32, s_asset_id::make_core_font_id(ec_core_font::eb_garamond_48));
    }
}

void c_renderer::draw(const c_assets &assets, const cc::s_vec_2d_i window_size, const s_camera &cam) const
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for (int i = 0; i < get_layer_cnt(); ++i)
    {
        const s_render_layer_info &layer_info = m_layer_infos[i];

        for (int j = 0; j < layer_info.sprite_batch_cnt; ++j)
        {
            const int batch_index = layer_info.begin_sprite_batch_index + j;
            m_sprite_batches[batch_index].draw(assets, window_size, i < m_sprite_batch_cam_layer_cnt ? &cam : nullptr);
        }

        for (int j = 0; j < layer_info.char_batch_cnt; ++j)
        {
            const int batch_index = layer_info.begin_char_batch_index + j;
            m_char_batches[batch_index].draw(assets, window_size);
        }
    }
}

s_sprite_batch_slot_key c_renderer::take_any_sprite_batch_slot(const int layer_index, const s_asset_id tex_id)
{
    assert(layer_index >= 0 && layer_index < get_layer_cnt());

    const s_render_layer_info &layer_info = m_layer_infos[layer_index];

    for (int i = 0; i < layer_info.sprite_batch_cnt; ++i)
    {
        const int batch_index = layer_info.begin_sprite_batch_index + i;
        const int slot_index = m_sprite_batches[batch_index].take_any_slot(tex_id);

        if (slot_index != -1)
        {
            return {layer_index, i, slot_index};
        }
    }

    add_sprite_batch_to_layer(layer_index);

    return take_any_sprite_batch_slot(layer_index, tex_id); // TEMP: We really only need to check the last batch.
}

s_char_batch_key c_renderer::add_char_batch_to_layer(const int layer_index, const int slot_cnt, const s_asset_id font_id)
{
    const int layer_batch_index = m_layer_infos[layer_index].char_batch_cnt;
    const int batch_index = m_layer_infos[layer_index].begin_char_batch_index + layer_batch_index;
    m_char_batches.emplace(m_char_batches.begin() + batch_index, slot_cnt, font_id);

    m_layer_infos[layer_index].char_batch_cnt++;

    // Update the begin indexes of subsequent layers.
    for (int i = layer_index + 1; i < m_layer_init_infos.size(); ++i)
    {
        m_layer_infos[i].begin_char_batch_index++;
    }

    return {layer_index, layer_batch_index};
}

void c_renderer::add_sprite_batch_to_layer(const int layer_index)
{
    const int batch_slot_cnt = m_layer_init_infos[layer_index].sprite_batch_default_slot_cnt;
    const int batch_index = m_layer_infos[layer_index].begin_sprite_batch_index + m_layer_infos[layer_index].sprite_batch_cnt;
    m_sprite_batches.emplace(m_sprite_batches.begin() + batch_index, batch_slot_cnt);

    m_layer_infos[layer_index].sprite_batch_cnt++;

    // Update the begin indexes of subsequent layers.
    for (int i = layer_index + 1; i < m_layer_init_infos.size(); ++i)
    {
        m_layer_infos[i].begin_sprite_batch_index++;
    }
}
