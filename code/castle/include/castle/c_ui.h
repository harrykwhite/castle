#pragma once

#include <castle/c_rendering.h>
#include <castle/c_assets.h>
#include <castle/c_input.h>
#include <castle/c_player.h>

struct s_ui
{
    s_sprite_batch_slot_key cursor_sb_slot_key;
    s_sprite_batch_slot_key player_sb_slot_key[k_player_inv_hotbar_slot_cnt];
};

s_ui make_ui(s_sprite_batch_collection &sb_collection);
void write_ui_render_data(const s_ui &ui, const s_sprite_batch_collection &sb_collection, const c_assets &assets, const s_input_state_pair &input_state_pair, const s_camera &cam, const cc::s_vec_2d_i window_size, const int player_inv_hotbar_slot_selected);
