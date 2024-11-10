#pragma once

#include "c_input.h"
#include "c_rendering.h"

constexpr float k_player_move_spd = 2.0f;

class c_player_ent
{
public:
    c_player_ent(c_sprite_batch_layer &sb_layer);

    void proc_movement(const c_input_manager &input_manager);
    void rewrite_render_data(c_sprite_batch_layer &sb_layer, const c_assets &assets);

private:
    s_sprite_batch_slot_key m_sb_slot_key;

    cc::s_vec_2d m_pos = {};
    cc::s_vec_2d m_vel = {};
};
