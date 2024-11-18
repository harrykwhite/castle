#pragma once

#include <functional>
#include "c_input.h"
#include "c_rendering.h"
#include "c_assets.h"
#include "c_tilemap.h"

constexpr int k_player_inv_hotbar_slot_cnt = 7;

constexpr float k_player_ent_move_spd = 2.0f;

using u_collider_maker = std::function<cc::s_rect_f(const cc::s_vec_2d pos)>;

struct s_player_ent
{
    s_sprite_batch_slot_key sb_slot_key;
    cc::s_vec_2d pos;
    cc::s_vec_2d vel;
    float rot;
    u_collider_maker collider_maker;

    s_player_ent(const s_sprite_batch_slot_key sb_slot_key, cc::s_vec_2d pos, cc::s_vec_2d vel, const float rot, const u_collider_maker collider_maker)
        : sb_slot_key(sb_slot_key), pos(pos), vel(vel), rot(rot), collider_maker(collider_maker)
    {
    }

    inline s_player_ent with_pos(const cc::s_vec_2d pos) const
    {
        return {sb_slot_key, pos, vel, rot, collider_maker};
    }

    inline s_player_ent with_vel(const cc::s_vec_2d vel) const
    {
        return {sb_slot_key, pos, vel, rot, collider_maker};
    }

    inline s_player_ent with_rot(const float rot) const
    {
        return {sb_slot_key, pos, vel, rot, collider_maker};
    }
};

s_player_ent make_player_ent(const cc::s_vec_2d pos, s_sprite_batch_collection &batch_collection, const c_assets &assets);
s_player_ent player_ent_after_tick(const s_player_ent &ent, const s_input_state_pair &input_state_pair, const c_tilemap &tilemap, const c_assets &assets, const s_camera &cam, const cc::s_vec_2d_i window_size);
void write_player_ent_render_data(const s_player_ent &player_ent, const s_sprite_batch_collection &sb_collection, const c_assets &assets);

cc::s_vec_2d vel_after_tile_collision_proc(const cc::s_vec_2d vel, const cc::s_vec_2d pos, const u_collider_maker collider_maker, const c_tilemap &tilemap);

inline u_collider_maker gen_collider_maker(const cc::s_vec_2d offs, const cc::s_vec_2d size)
{
    assert(size.x > 0.0f && size.y > 0.0f);

    return [offs, size](const cc::s_vec_2d pos)
    {
        return cc::s_rect_f {pos.x + offs.x, pos.y + offs.y, size.x, size.y};
    };
}

constexpr s_asset_id k_player_ent_tex_id = s_asset_id::make_core_tex_id(ec_core_tex::player);
constexpr cc::s_vec_2d k_player_ent_origin = {0.5f, 0.5f};
