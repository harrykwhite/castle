#include <castle/c_rendering.h>

#include <castle/c_assets.h>
#include <castle_common/cc_misc.h>

c_sprite_batch::c_sprite_batch()
{
    // Generate vertex array.
    glGenVertexArrays(1, &m_vert_array_gl_id);
    glBindVertexArray(m_vert_array_gl_id);

    // Generate vertex buffer.
    glGenBuffers(1, &m_vert_buf_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_vert_buf_gl_id);

    {
        const int verts_len = k_sprite_quad_shader_prog_vert_cnt * 4 * c_sprite_batch::k_slot_cnt;
        const auto verts = std::make_unique<const float[]>(verts_len);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts[0]) * verts_len, verts.get(), GL_DYNAMIC_DRAW);
    }

    // Generate element buffer.
    glGenBuffers(1, &m_elem_buf_gl_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elem_buf_gl_id);

    {
        const int indices_len = 6 * c_sprite_batch::k_slot_cnt;
        const auto indices = std::make_unique<unsigned short[]>(indices_len); // TODO: Have it so that the indices are only set up once; all batches use a copy of this same data, so recalculation isn't needed.

        for (int i = 0; i < c_sprite_batch::k_slot_cnt; i++)
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

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, verts_stride, reinterpret_cast<void *>(sizeof(float) * 0));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, verts_stride, reinterpret_cast<void *>(sizeof(float) * 2));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, verts_stride, reinterpret_cast<void *>(sizeof(float) * 4));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, verts_stride, reinterpret_cast<void *>(sizeof(float) * 6));
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, verts_stride, reinterpret_cast<void *>(sizeof(float) * 7));
    glEnableVertexAttribArray(4);

    glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, verts_stride, reinterpret_cast<void *>(sizeof(float) * 8));
    glEnableVertexAttribArray(5);

    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, verts_stride, reinterpret_cast<void *>(sizeof(float) * 10));
    glEnableVertexAttribArray(6);

    glBindVertexArray(0);
}

c_sprite_batch::~c_sprite_batch()
{
    glDeleteBuffers(1, &m_elem_buf_gl_id);
    glDeleteBuffers(1, &m_vert_buf_gl_id);
    glDeleteVertexArrays(1, &m_vert_array_gl_id);
}

int c_sprite_batch::take_any_available_slot(const s_asset_id tex_id)
{
    // If there are no free/inactive slots, don't go any further.
    if (m_slot_activity.is_full())
    {
        return -1;
    }

    // Try to find a texture unit to use, don't go any further if not found.
    const int tex_unit = find_tex_unit_to_use(tex_id);

    if (tex_unit == -1)
    {
        return -1;
    }

    // Pop a slot index from the available slot stack if not empty, otherwise manually find a free slot.
    int slot_index;

    if (m_available_slot_stack_size > 0)
    {
        slot_index = m_available_slot_stack[m_available_slot_stack_size - 1];
        --m_available_slot_stack_size;
    }
    else
    {
        slot_index = m_slot_activity.get_first_inactive_bit_index();
    }

    // Mark the slot as active.
    m_slot_activity.activate_bit(slot_index);

    // Update texture unit information.
    ++m_tex_unit_ref_cnts[tex_unit];
    m_tex_unit_tex_ids[tex_unit] = tex_id;
    m_slot_tex_units[slot_index] = tex_unit;

    return slot_index;
}

void c_sprite_batch::write_to_slot(const int slot_index, const s_sprite_batch_slot_write_data &write_data, const c_assets &assets)
{
    CC_CHECK(slot_index >= 0 && slot_index < k_slot_cnt);

    const int tex_unit = m_slot_tex_units[slot_index];
    const cc::s_vec_2d_int tex_size = assets.get_tex_size(m_tex_unit_tex_ids[tex_unit]);

    const float verts[k_sprite_quad_shader_prog_vert_cnt * 4] = {
        (0.0f - write_data.origin.x) * write_data.scale.x,
        (0.0f - write_data.origin.y) * write_data.scale.y,
        write_data.pos.x,
        write_data.pos.y,
        static_cast<float>(write_data.src_rect.width),
        static_cast<float>(write_data.src_rect.height),
        write_data.rot,
        static_cast<float>(tex_unit),
        static_cast<float>(write_data.src_rect.x) / tex_size.x,
        static_cast<float>(write_data.src_rect.y) / tex_size.y,
        write_data.blend.r,
        write_data.blend.g,
        write_data.blend.b,
        write_data.blend.a,

        (1.0f - write_data.origin.x) * write_data.scale.x,
        (0.0f - write_data.origin.y) * write_data.scale.y,
        write_data.pos.x,
        write_data.pos.y,
        static_cast<float>(write_data.src_rect.width),
        static_cast<float>(write_data.src_rect.height),
        write_data.rot,
        static_cast<float>(tex_unit),
        static_cast<float>(write_data.src_rect.x + write_data.src_rect.width) / tex_size.x,
        static_cast<float>(write_data.src_rect.y) / tex_size.y,
        write_data.blend.r,
        write_data.blend.g,
        write_data.blend.b,
        write_data.blend.a,

        (1.0f - write_data.origin.x) * write_data.scale.x,
        (1.0f - write_data.origin.y) * write_data.scale.y,
        write_data.pos.x,
        write_data.pos.y,
        static_cast<float>(write_data.src_rect.width),
        static_cast<float>(write_data.src_rect.height),
        write_data.rot,
        static_cast<float>(tex_unit),
        static_cast<float>(write_data.src_rect.x + write_data.src_rect.width) / tex_size.x,
        static_cast<float>(write_data.src_rect.y + write_data.src_rect.height) / tex_size.y,
        write_data.blend.r,
        write_data.blend.g,
        write_data.blend.b,
        write_data.blend.a,

        (0.0f - write_data.origin.x) * write_data.scale.x,
        (1.0f - write_data.origin.y) * write_data.scale.y,
        write_data.pos.x,
        write_data.pos.y,
        static_cast<float>(write_data.src_rect.width),
        static_cast<float>(write_data.src_rect.height),
        write_data.rot,
        static_cast<float>(tex_unit),
        static_cast<float>(write_data.src_rect.x) / tex_size.x,
        static_cast<float>(write_data.src_rect.y + write_data.src_rect.height) / tex_size.y,
        write_data.blend.r,
        write_data.blend.g,
        write_data.blend.b,
        write_data.blend.a
    };

    glBindVertexArray(m_vert_array_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_vert_buf_gl_id);
    glBufferSubData(GL_ARRAY_BUFFER, slot_index * sizeof(verts), sizeof(verts), verts);
}

void c_sprite_batch::release_slot(const int slot_index)
{
    CC_CHECK(slot_index >= 0 && slot_index < k_slot_cnt);

    // Mark the slot as inactive.
    m_slot_activity.deactivate_bit(slot_index);

    // Update texture unit information.
    const int tex_unit = m_slot_tex_units[slot_index];
    --m_tex_unit_ref_cnts[tex_unit];

    // Clear the vertex data.
    glBindVertexArray(m_vert_array_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_vert_buf_gl_id);

    const float verts[k_sprite_quad_shader_prog_vert_cnt * 4] = {};
    glBufferSubData(GL_ARRAY_BUFFER, slot_index * sizeof(verts), sizeof(verts), verts);
}

void c_sprite_batch::render(const c_assets &assets, const cc::s_vec_2d_int window_size) const
{
    const int sprite_quad_prog_gl_id = assets.get_shader_prog_gl_id({0, 0});

    glUseProgram(sprite_quad_prog_gl_id);

    const cc::s_matrix_4x4 view = cc::s_matrix_4x4::make_identity();
    glUniformMatrix4fv(glGetUniformLocation(sprite_quad_prog_gl_id, "u_view"), 1, GL_FALSE, reinterpret_cast<const float *>(view.elems));

    const cc::s_matrix_4x4 proj = cc::s_matrix_4x4::make_ortho(0.0f, window_size.x, window_size.y, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(sprite_quad_prog_gl_id, "u_proj"), 1, GL_FALSE, reinterpret_cast<const float *>(proj.elems));

    // Set up texture units.
    int tex_units[k_tex_unit_limit] = {0};

    for (int i = 0; i < k_tex_unit_limit; ++i)
    {
        tex_units[i] = i;

        if (m_tex_unit_ref_cnts[i] > 0)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, assets.get_tex_gl_id(m_tex_unit_tex_ids[i]));
        }
    }

    glUniform1iv(glGetUniformLocation(sprite_quad_prog_gl_id, "u_textures"), CC_STATIC_ARRAY_LEN(tex_units), tex_units);

    // Draw the batch.
    glBindVertexArray(m_vert_array_gl_id);
    glDrawElements(GL_TRIANGLES, 6 * k_slot_cnt, GL_UNSIGNED_SHORT, 0);
}

int c_sprite_batch::find_tex_unit_to_use(const s_asset_id tex_id) const
{
    int free_tex_unit = -1;

    for (int i = 0; i < k_tex_unit_limit; ++i)
    {
        if (!m_tex_unit_ref_cnts[i])
        {
            free_tex_unit = i;
            continue;
        }

        if (m_tex_unit_tex_ids[i] == tex_id)
        {
            return i;
        }
    }

    return free_tex_unit;
}

s_sprite_batch_slot_key c_sprite_batch_layer::take_any_available_slot(const s_asset_id tex_id)
{
    s_sprite_batch_slot_key key;

    const auto take_slot_and_update_key = [&key, tex_id, this](const int batch_index)
    {
        const int slot_index_taken = m_batches[batch_index].take_any_available_slot(tex_id);

        if (slot_index_taken != -1)
        {
            key.batch_index = batch_index;
            key.slot_index = slot_index_taken;
            return true;
        }

        return false;
    };

    for (int i = 0; i < m_batches.size(); ++i)
    {
        if (take_slot_and_update_key(i))
        {
            return key;
        }
    }

    m_batches.emplace_back();
    static_cast<void>(take_slot_and_update_key(m_batches.size() - 1));
    return key;
}

void c_sprite_batch_layer::render(const c_assets &assets, const cc::s_vec_2d_int window_size) const
{
    for (const auto &batch : m_batches)
    {
        batch.render(assets, window_size);
    }
}
