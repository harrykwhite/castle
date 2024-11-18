#include <castle/c_rendering.h>

#include <array>
#include <castle/c_assets.h>
#include <castle_common/cc_misc.h>

static s_sprite_batch make_sprite_batch()
{
    s_sprite_batch batch = {};

    // Generate vertex array.
    glGenVertexArrays(1, &batch.vert_array_gl_id);
    glBindVertexArray(batch.vert_array_gl_id);

    // Generate vertex buffer.
    glGenBuffers(1, &batch.vert_buf_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, batch.vert_buf_gl_id);

    {
        const int verts_len = k_sprite_quad_shader_prog_vert_cnt * 4 * s_sprite_batch::k_slot_cnt;
        const auto verts = std::make_unique<const float[]>(verts_len);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts[0]) * verts_len, verts.get(), GL_DYNAMIC_DRAW);
    }

    // Generate element buffer.
    glGenBuffers(1, &batch.elem_buf_gl_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch.elem_buf_gl_id);

    {
        // TODO: Have it so that the indices are only set up once; all batches use a copy of this same data, so recalculation isn't needed.
        const int indices_len = 6 * s_sprite_batch::k_slot_cnt;
        const auto indices = std::make_unique<unsigned short[]>(indices_len); // TODO: Have it so that the indices are only set up once; all batches use a copy of this same data, so recalculation isn't needed.

        for (int i = 0; i < s_sprite_batch::k_slot_cnt; i++)
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

    return batch;
}

static inline cc::s_matrix_4x4 make_camera_view_matrix(const s_camera &cam, const cc::s_vec_2d_i window_size)
{
    auto mat = cc::s_matrix_4x4::identity();
    mat[0][0] = cam.scale;
    mat[1][1] = cam.scale;
    mat[3][0] = (-cam.pos.x * cam.scale) + (window_size.x / 2.0f);
    mat[3][1] = (-cam.pos.y * cam.scale) + (window_size.y / 2.0f);
    return mat;
}

static int find_sprite_batch_tex_unit_to_use(const s_sprite_batch &batch, const s_asset_id tex_id)
{
    int free_tex_unit = -1;

    for (int i = 0; i < s_sprite_batch::k_tex_unit_limit; ++i)
    {
        if (!batch.tex_unit_ref_cnts[i])
        {
            free_tex_unit = i;
            continue;
        }

        if (batch.tex_unit_tex_ids[i] == tex_id)
        {
            return i;
        }
    }

    return free_tex_unit;
}

s_sprite_batch_collection make_sprite_batch_collection(const std::vector<int> &layer_batch_cnts, const int screen_layers_begin_batch_index)
{
    const int layer_cnt = layer_batch_cnts.size();

    assert(layer_cnt >= 1);
    assert(screen_layers_begin_batch_index >= 0 && screen_layers_begin_batch_index <= layer_cnt);

    // Set the overall batch count to the sum of all layer batch counts.
    const int batch_cnt = std::accumulate(layer_batch_cnts.begin(), layer_batch_cnts.end(), 0);

    // Create the buffer.
    const int buf_size = (batch_cnt * sizeof(s_sprite_batch)) + (layer_cnt * sizeof(s_sprite_batch_layer_info));
    auto const buf = new cc::u_byte[buf_size]();

    // Determine buffer pointers.
    auto const buf_batches = reinterpret_cast<s_sprite_batch *>(buf);
    auto const buf_layer_infos = reinterpret_cast<s_sprite_batch_layer_info *>(buf_batches + batch_cnt);

    // Set up the batches.
    for (int i = 0; i < batch_cnt; ++i)
    {
        buf_batches[i] = make_sprite_batch();
    }

    // Set layer information structs.
    buf_layer_infos[0].batch_cnt = layer_batch_cnts[0];
    buf_layer_infos[0].begin_batch_index = 0;

    for (int i = 1; i < layer_cnt; ++i)
    {
        buf_layer_infos[i].batch_cnt = layer_batch_cnts[i];
        buf_layer_infos[i].begin_batch_index = buf_layer_infos[i - 1].begin_batch_index + buf_layer_infos[i - 1].batch_cnt;
    }

    return {
        layer_cnt,
        batch_cnt,
        buf,
        buf_batches,
        buf_layer_infos,
        screen_layers_begin_batch_index
    };
}

void dispose_sprite_batch_collection(const s_sprite_batch_collection &collection)
{
    for (int i = 0; i < collection.batch_cnt; ++i)
    {
        const s_sprite_batch &batch = collection.buf_batches[i];

        glDeleteBuffers(1, &batch.elem_buf_gl_id);
        glDeleteBuffers(1, &batch.vert_buf_gl_id);
        glDeleteVertexArrays(1, &batch.vert_array_gl_id);
    }

    delete[] collection.buf;
}

void render_sprite_batches(const s_sprite_batch_collection &batch_collection, const s_camera &cam, const c_assets &assets, const cc::s_vec_2d_i window_size)
{
    const int sprite_quad_prog_gl_id = assets.get_shader_prog_gl_id(s_asset_id::make_core_shader_prog_id(ec_core_shader_prog::sprite_quad));
    glUseProgram(sprite_quad_prog_gl_id);

    // Set up texture units.
    const auto tex_units = []()
    {
        std::array<int, s_sprite_batch::k_tex_unit_limit> tex_units;
        std::iota(tex_units.begin(), tex_units.end(), 0);
        return tex_units;
    }();

    glUniform1iv(glGetUniformLocation(sprite_quad_prog_gl_id, "u_textures"), tex_units.size(), tex_units.data());

    // Set up the projection matrix.
    const auto proj_mat = cc::s_matrix_4x4::ortho(0.0f, window_size.x, window_size.y, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(sprite_quad_prog_gl_id, "u_proj"), 1, GL_FALSE, reinterpret_cast<const float *>(proj_mat.elems));

    // Define function for drawing batches in range.
    auto draw_batches = [&batch_collection, &assets](const int begin_index, const int end_index)
    {
        assert(begin_index >= 0 && begin_index < batch_collection.batch_cnt);
        assert(end_index >= 0 && end_index <= batch_collection.batch_cnt);
        assert(begin_index <= end_index);

        for (int i = begin_index; i < end_index; ++i)
        {
            const s_sprite_batch &batch = batch_collection.buf_batches[i];
            
            for (int j = 0; j < s_sprite_batch::k_tex_unit_limit; ++j)
            {
                if (batch.tex_unit_ref_cnts[j] > 0)
                {
                    glActiveTexture(GL_TEXTURE0 + j);
                    glBindTexture(GL_TEXTURE_2D, assets.get_tex_gl_id(batch.tex_unit_tex_ids[j]));
                }
            }

            glBindVertexArray(batch.vert_array_gl_id);
            glDrawElements(GL_TRIANGLES, 6 * s_sprite_batch::k_slot_cnt, GL_UNSIGNED_SHORT, NULL);
        }
    };

    // Draw camera batches.
    const cc::s_matrix_4x4 view_mat_cam = make_camera_view_matrix(cam, window_size);
    glUniformMatrix4fv(glGetUniformLocation(sprite_quad_prog_gl_id, "u_view"), 1, GL_FALSE, reinterpret_cast<const float *>(view_mat_cam.elems));

    draw_batches(0, batch_collection.screen_layers_begin_batch_index);

    // Draw screen batches.
    const cc::s_matrix_4x4 view_mat_screen = cc::s_matrix_4x4::identity();
    glUniformMatrix4fv(glGetUniformLocation(sprite_quad_prog_gl_id, "u_view"), 1, GL_FALSE, reinterpret_cast<const float *>(view_mat_screen.elems));

    draw_batches(batch_collection.screen_layers_begin_batch_index, batch_collection.batch_cnt);
}

s_sprite_batch_slot_key take_any_sprite_batch_slot(const int layer_index, const s_asset_id tex_id, s_sprite_batch_collection &batch_collection)
{
    for (int i = 0; i < batch_collection.buf_layer_infos[layer_index].batch_cnt; ++i)
    {
        const int batch_index = batch_collection.buf_layer_infos[layer_index].begin_batch_index + i;
        s_sprite_batch &batch = batch_collection.buf_batches[batch_index];

        // If there are no free/inactive slots, don't go any further.
        if (batch.slot_activity.is_full())
        {
            continue;
        }

        // Try to find a texture unit to use, and don't go any further if not found.
        const int tex_unit = find_sprite_batch_tex_unit_to_use(batch, tex_id);

        if (tex_unit == -1)
        {
            continue;
        }

        // Pop a slot index from the available slot stack if not empty, otherwise manually find a free slot.
        const int slot_index = [&batch]()
        {
            if (batch.available_slot_stack_size > 0)
            {
                --batch.available_slot_stack_size;
                return batch.available_slot_stack[batch.available_slot_stack_size];
            }

            return batch.slot_activity.get_first_inactive_bit_index();
        }();

        // Mark the slot as active.
        batch.slot_activity.activate_bit(slot_index);

        // Update texture unit information.
        ++batch.tex_unit_ref_cnts[tex_unit];
        batch.tex_unit_tex_ids[tex_unit] = tex_id;
        batch.slot_tex_units[slot_index] = tex_unit;

        return {batch_index, slot_index};
    }

    // Failed to take a slot.
    assert(false);
    return {};
}

void write_to_sprite_batch_slot(const s_sprite_batch_slot_key slot_key, const s_sprite_batch_collection &batch_collection, const c_assets &assets, const cc::s_vec_2d pos, const cc::s_rect &src_rect, const cc::s_vec_2d origin, const float rot, const cc::s_vec_2d scale, const s_color &blend)
{
    const s_sprite_batch &batch = batch_collection.buf_batches[slot_key.batch_index];

    const int tex_unit = batch.slot_tex_units[slot_key.slot_index];
    const cc::s_vec_2d_i tex_size = assets.get_tex_size(batch.tex_unit_tex_ids[tex_unit]);

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
        blend.r, // NOTE: We might not even need this...
        blend.g, // NOTE: We might not even need this...
        blend.b, // NOTE: We might not even need this...
        blend.a,

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
        blend.r,
        blend.g,
        blend.b,
        blend.a,

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
        blend.r,
        blend.g,
        blend.b,
        blend.a,

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
        blend.r,
        blend.g,
        blend.b,
        blend.a
    };

    glBindVertexArray(batch.vert_array_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, batch.vert_buf_gl_id);
    glBufferSubData(GL_ARRAY_BUFFER, slot_key.slot_index * sizeof(verts), sizeof(verts), verts);
}

void clear_sprite_batch_slot(const s_sprite_batch_slot_key slot_key, const s_sprite_batch_collection &batch_collection)
{
    const s_sprite_batch &batch = batch_collection.buf_batches[slot_key.batch_index];

    glBindVertexArray(batch.vert_array_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, batch.vert_buf_gl_id);

    const float verts[k_sprite_quad_shader_prog_vert_cnt * 4] = {};
    glBufferSubData(GL_ARRAY_BUFFER, slot_key.slot_index * sizeof(verts), sizeof(verts), verts);
}

void release_sprite_batch_slot(const s_sprite_batch_slot_key slot_key, s_sprite_batch_collection &batch_collection)
{
    s_sprite_batch &batch = batch_collection.buf_batches[slot_key.batch_index];

    // Mark the slot as inactive.
    batch.slot_activity.deactivate_bit(slot_key.slot_index);

    // Update texture unit information.
    const int tex_unit = batch.slot_tex_units[slot_key.slot_index];
    --batch.tex_unit_ref_cnts[tex_unit];

    // Clear the slot render data.
    clear_sprite_batch_slot(slot_key, batch_collection);
}
