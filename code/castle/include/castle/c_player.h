#pragma once

#include "c_input.h"
#include "c_rendering.h"
#include "c_assets.h"
#include "c_tilemap.h"

constexpr s_asset_id k_player_tex_id = s_asset_id::make_core_tex_id(ec_core_tex::player);
constexpr cc::s_vec_2d k_player_origin = {0.5f, 0.5f};
constexpr float k_player_move_spd = 2.0f;

class c_player_ent
{
public:
    c_player_ent(c_renderer &renderer);

    void proc_movement(const c_input_manager &input_manager, const c_tilemap &tilemap, const c_assets &assets);
    void rewrite_render_data(const c_renderer &renderer, const c_assets &assets);

    inline cc::s_rect_float get_collider(const c_assets &assets, const cc::s_vec_2d offs = {}) const
    {
        const cc::s_vec_2d_int tex_size = assets.get_tex_size(k_player_tex_id);
        return {m_pos.x - (k_player_origin.x * tex_size.x) + offs.x, m_pos.y - (k_player_origin.y * tex_size.y) + offs.y, static_cast<float>(tex_size.x), static_cast<float>(tex_size.y)};
    }

private:
    s_sprite_batch_slot_key m_sb_slot_key;

    cc::s_vec_2d m_pos = {};
    cc::s_vec_2d m_vel = {};

    float m_rot = 0.0f;

    void proc_hor_and_ver_tile_collisions(const c_tilemap &tilemap, const c_assets &assets);
};
