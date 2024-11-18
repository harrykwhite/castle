#pragma once

#include <vector>
#include <numeric>
#include <cassert>
#include <castle_common/cc_math.h>
#include "c_assets.h"
#include "c_utils.h"

enum class ec_sprite_batch_layer
{
    player,
    tiles,
    player_inv_hotbar,
    cursor
};

struct s_color
{
    static constexpr s_color white() { return {1.0f, 1.0f, 1.0f, 1.0f}; }
    static constexpr s_color black() { return {0.0f, 0.0f, 0.0f, 1.0f}; }
    static constexpr s_color red() { return {1.0f, 0.0f, 0.0f, 1.0f}; }
    static constexpr s_color green() { return {0.0f, 1.0f, 0.0f, 1.0f}; }
    static constexpr s_color blue() { return {0.0f, 0.0f, 1.0f, 1.0f}; }
    static constexpr s_color yellow() { return {1.0f, 1.0f, 0.0f, 1.0f}; }
    static constexpr s_color cyan() { return {0.0f, 1.0f, 1.0f, 1.0f}; }
    static constexpr s_color magenta() { return {1.0f, 0.0f, 1.0f, 1.0f}; }

    float r, g, b, a;
};

struct s_camera
{
    cc::s_vec_2d pos;
    static constexpr float scale = 2.0f; // TEMP: This will likely be modifiable in an options menu in the future.
};

struct s_sprite_batch
{
    static constexpr int k_slot_cnt = 1024;
    static constexpr int k_available_slot_stack_size_max = 32;
    static constexpr int k_tex_unit_limit = 32; // TODO: Have the real texture unit limit have an effect.

    u_gl_id vert_array_gl_id;
    u_gl_id vert_buf_gl_id;
    u_gl_id elem_buf_gl_id;

    c_bitset<k_slot_cnt> slot_activity;

    // This is a stack where the indexes of released slots are pushed, so that when a new slot needs to be taken an available index can be taken from the top here.
    int available_slot_stack[k_available_slot_stack_size_max];
    int available_slot_stack_size;

    int slot_tex_units[k_slot_cnt]; // What texture unit each slot is mapped to.

    // NOTE: Consider bundling the below into a struct.
    s_asset_id tex_unit_tex_ids[k_tex_unit_limit]; // What texture ID (the actual texture asset) each unit maps to.
    int tex_unit_ref_cnts[k_tex_unit_limit]; // How many slots are mapped to each unit.
};

struct s_sprite_batch_layer_info
{
    int batch_cnt;
    int begin_batch_index;
};

struct s_sprite_batch_collection
{
    const int layer_cnt;
    const int batch_cnt;

    const cc::u_byte *const buf;
    s_sprite_batch *const buf_batches;
    const s_sprite_batch_layer_info *const buf_layer_infos;

    const int screen_layers_begin_batch_index;
};

struct s_sprite_batch_slot_key
{
    int batch_index;
    int slot_index;
#if 0
    const int batch_index;
    const int slot_index;
#endif
};

s_sprite_batch_collection make_sprite_batch_collection(const std::vector<int> &layer_batch_cnts, const int screen_layers_begin_batch_index);
void dispose_sprite_batch_collection(const s_sprite_batch_collection &collection);
void render_sprite_batches(const s_sprite_batch_collection &batch_collection, const s_camera &cam, const c_assets &assets, const cc::s_vec_2d_i window_size);
s_sprite_batch_slot_key take_any_sprite_batch_slot(const int layer_index, const s_asset_id tex_id, s_sprite_batch_collection &batch_collection);
void write_to_sprite_batch_slot(const s_sprite_batch_slot_key slot_key, const s_sprite_batch_collection &batch_collection, const c_assets &assets, const cc::s_vec_2d pos, const cc::s_rect &src_rect, const cc::s_vec_2d origin = {}, const float rot = 0.0f, const cc::s_vec_2d scale = {1.0f, 1.0f}, const s_color &blend = s_color::white());
void clear_sprite_batch_slot(const s_sprite_batch_slot_key slot_key, const s_sprite_batch_collection &batch_collection);
void release_sprite_batch_slot(const s_sprite_batch_slot_key slot_key, s_sprite_batch_collection &batch_collection);

inline cc::s_vec_2d cam_to_screen_pos(const cc::s_vec_2d pos, const s_camera &cam, const cc::s_vec_2d_i window_size)
{
    return {
        ((pos.x - cam.pos.x) * cam.scale) + (window_size.x / 2.0f),
        ((pos.y - cam.pos.y) * cam.scale) + (window_size.y / 2.0f)
    };
}

inline cc::s_vec_2d screen_to_cam_pos(const cc::s_vec_2d pos, const s_camera &cam, const cc::s_vec_2d_i window_size)
{
    return {
        ((pos.x - (window_size.x / 2.0f)) / cam.scale) + cam.pos.x,
        ((pos.y - (window_size.y / 2.0f)) / cam.scale) + cam.pos.y
    };
}

constexpr int k_sprite_quad_shader_prog_vert_cnt = 14;
