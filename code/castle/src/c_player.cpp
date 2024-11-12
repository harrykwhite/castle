#include <castle/c_player.h>

c_player_ent::c_player_ent(c_sprite_batch_layer &sb_layer)
{
    m_sb_slot_key = sb_layer.take_any_available_slot(s_asset_id::make_core_tex_id(ec_core_tex::player));
}

void c_player_ent::proc_movement(const c_input_manager &input_manager, const c_tilemap &tilemap, const c_assets &assets)
{
    const cc::s_vec_2d move_axis = {
        input_manager.is_key_down(ec_key_code::d) - input_manager.is_key_down(ec_key_code::a),
        input_manager.is_key_down(ec_key_code::s) - input_manager.is_key_down(ec_key_code::w)
    };

    m_vel = move_axis * k_player_move_spd;

    proc_hor_and_ver_tile_collisions(tilemap, assets);

    m_pos += m_vel;
}

void c_player_ent::rewrite_render_data(c_sprite_batch_layer &sb_layer, const c_assets &assets)
{
    s_sprite_batch_slot_write_data write_data = {};
    write_data.pos = m_pos;
    write_data.scale = {1.0f, 1.0f};
    write_data.origin = {0.5f, 0.5f};
    write_data.src_rect.width = 32.0f;
    write_data.src_rect.height = 32.0f;
    write_data.blend = s_color::make_white();

    sb_layer.write_to_slot(m_sb_slot_key, write_data, assets);
}

void c_player_ent::proc_hor_and_ver_tile_collisions(const c_tilemap &tilemap, const c_assets &assets)
{
    const cc::s_rect_float hor_collider = get_collider(assets, {m_vel.x, 0.0f});
    const cc::s_rect_float ver_collider = get_collider(assets, {0.0f, m_vel.y});

    unsigned char collision_flags = 0; // The first bit is set when a horizontal collision occurs, and the second bit is set when a vertical collision occurs.

    for (int ty = 0; ty < k_tilemap_size; ++ty)
    {
        for (int tx = 0; tx < k_tilemap_size; ++tx)
        {
            if (!tilemap.is_tile_active(tx, ty))
            {
                continue;
            }

            const cc::s_rect_float tile_collider = tilemap.get_tile_collider(tx, ty);

            if (!(collision_flags & 1) && hor_collider.intersects(tile_collider))
            {
                m_vel.x = 0.0f;
                collision_flags |= 1;

                if (collision_flags == 3)
                {
                    return;
                }
            }

            if (!(collision_flags & 2) && ver_collider.intersects(tile_collider))
            {
                m_vel.y = 0.0f;
                collision_flags |= 2;

                if (collision_flags == 3)
                {
                    return;
                }
            }
        }
    }
}
