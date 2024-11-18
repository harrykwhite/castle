#include <castle/c_player.h>

s_player_ent make_player_ent(const cc::s_vec_2d pos, s_sprite_batch_collection &batch_collection, const c_assets &assets)
{
    const cc::s_vec_2d_i tex_size = assets.get_tex_size(k_player_ent_tex_id);
    const cc::s_vec_2d tex_size_f = {static_cast<float>(tex_size.x), static_cast<float>(tex_size.y)};

    return {
        take_any_sprite_batch_slot(static_cast<int>(ec_sprite_batch_layer::player), s_asset_id::make_core_tex_id(ec_core_tex::player), batch_collection),
        pos,
        {},
        0.0f,
        gen_collider_maker(-cc::s_vec_2d {tex_size_f.x * k_player_ent_origin.x, tex_size_f.y * k_player_ent_origin.y}, tex_size_f)
    };
}

s_player_ent player_ent_after_tick(const s_player_ent &ent, const s_input_state_pair &input_state_pair, const c_tilemap &tilemap, const c_assets &assets, const s_camera &cam, const cc::s_vec_2d_i window_size)
{
    const cc::s_vec_2d move_axis = {
        static_cast<float>(is_key_down(ec_key_code::d, input_state_pair)) - static_cast<float>(is_key_down(ec_key_code::a, input_state_pair)),
        static_cast<float>(is_key_down(ec_key_code::s, input_state_pair)) - static_cast<float>(is_key_down(ec_key_code::w, input_state_pair))
    };
    const cc::s_vec_2d move_vel = move_axis * k_player_move_spd;
    const cc::s_vec_2d move_vel_after_tile_collisions = vel_after_tile_collision_proc(move_vel, ent.pos, ent.collider_maker, tilemap);
    const s_player_ent player_ent_after_move = ent.with_pos(ent.pos + move_vel_after_tile_collisions);

    const cc::s_vec_2d rot_targ = screen_to_cam_pos(input_state_pair.state.mouse_pos, cam, window_size);
    const s_player_ent player_ent_after_rot = player_ent_after_move.with_rot(cc::get_dir(player_ent_after_move.pos, rot_targ));

    return player_ent_after_rot;
}

void write_player_ent_render_data(const s_player_ent &player_ent, const s_sprite_batch_collection &sb_collection, const c_assets &assets)
{
    const cc::s_vec_2d_i tex_size = assets.get_tex_size(k_player_ent_tex_id);
    write_to_sprite_batch_slot(player_ent.sb_slot_key, sb_collection, assets, player_ent.pos, {0, 0, tex_size.x, tex_size.y}, k_player_ent_origin, player_ent.rot);
}

static cc::s_vec_2d vel_after_hor_and_ver_tile_collision_proc(const cc::s_vec_2d vel, const cc::s_vec_2d pos, const u_collider_maker collider_maker, const c_tilemap &tilemap)
{
    u_byte collision_flags = 0; // The first bit is set when a horizontal collision occurs, and the second bit is set when a vertical collision occurs.

    const cc::s_rect_f hor_collider = collider_maker(pos + cc::s_vec_2d {vel.x, 0.0f});
    const cc::s_rect_f ver_collider = collider_maker(pos + cc::s_vec_2d {0.0f, vel.y});
    
    for (int ty = 0; ty < k_tilemap_size; ++ty)
    {
        for (int tx = 0; tx < k_tilemap_size; ++tx)
        {
            if (!tilemap.is_tile_active(tx, ty))
            {
                continue;
            }

            const cc::s_rect_f tile_collider = tilemap.get_tile_collider(tx, ty);

            if (!(collision_flags & 1) && hor_collider.intersects(tile_collider))
            {
                collision_flags |= 1;

                if (collision_flags == 3)
                {
                    return {};
                }
            }

            if (!(collision_flags & 2) && ver_collider.intersects(tile_collider))
            {
                collision_flags |= 2;

                if (collision_flags == 3)
                {
                    return {};
                }
            }
        }
    }

    if (collision_flags == 1)
    {
        return {0.0f, vel.y};
    }

    if (collision_flags == 2)
    {
        return {vel.x, 0.0f};
    }

    return vel;
}

static cc::s_vec_2d vel_after_diag_tile_collision_proc(const cc::s_vec_2d vel, const cc::s_vec_2d pos, const u_collider_maker collider_maker, const c_tilemap &tilemap)
{
    if (vel.x == 0.0f)
    {
        // The diagonal collision check only affects the horizontal velocity. It is already 0, so there is no point in continuing.
        return vel;
    }

    const cc::s_rect_f diag_collider = collider_maker(pos + vel);

    for (int ty = 0; ty < k_tilemap_size; ++ty)
    {
        for (int tx = 0; tx < k_tilemap_size; ++tx)
        {
            if (!tilemap.is_tile_active(tx, ty))
            {
                continue;
            }

            const cc::s_rect_f tile_collider = tilemap.get_tile_collider(tx, ty);

            if (diag_collider.intersects(tile_collider))
            {
                return {0.0f, vel.y};
            }
        }
    }

    return vel;
}

cc::s_vec_2d vel_after_tile_collision_proc(const cc::s_vec_2d vel, const cc::s_vec_2d pos, const u_collider_maker collider_maker, const c_tilemap &tilemap)
{
    const cc::s_vec_2d hor_and_ver_proc_vel = vel_after_hor_and_ver_tile_collision_proc(vel, pos, collider_maker, tilemap);
    const cc::s_vec_2d diag_proc_vel = vel_after_diag_tile_collision_proc(hor_and_ver_proc_vel, pos, collider_maker, tilemap);
    return diag_proc_vel;
}
