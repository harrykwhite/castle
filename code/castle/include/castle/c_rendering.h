#pragma once

#include <castle_common/cc_math.h>
#include "c_assets.h"
#include "c_utils.h"
#include <stack>

struct s_color
{
    static constexpr s_color make_white() { return {1.0f, 1.0f, 1.0f, 1.0f}; }
    static constexpr s_color make_black() { return {0.0f, 0.0f, 0.0f, 1.0f}; }
    static constexpr s_color make_red() { return {1.0f, 0.0f, 0.0f, 1.0f}; }
    static constexpr s_color make_green() { return {0.0f, 1.0f, 0.0f, 1.0f}; }
    static constexpr s_color make_blue() { return {0.0f, 0.0f, 1.0f, 1.0f}; }
    static constexpr s_color make_yellow() { return {1.0f, 1.0f, 0.0f, 1.0f}; }
    static constexpr s_color make_cyan() { return {0.0f, 1.0f, 1.0f, 1.0f}; }
    static constexpr s_color make_magenta() { return {1.0f, 0.0f, 1.0f, 1.0f}; }

    float r, g, b, a;
};

struct s_sprite_batch_slot_key
{
    int layer_index;
    int batch_index;
    int slot_index;
};

struct s_sprite_batch_slot_write_data // NOTE: Consider removing this; parameters might be simpler.
{
    cc::s_vec_2d pos;
    float rot;
    cc::s_vec_2d scale;
    cc::s_vec_2d origin;
    cc::s_rect src_rect;
    s_color blend;
};

class c_sprite_batch
{
public:
    c_sprite_batch();
    ~c_sprite_batch();

    int take_any_available_slot(const s_asset_id tex_id);
    void write_to_slot(const int slot_index, const s_sprite_batch_slot_write_data &write_data, const c_assets &assets) const;
    void clear_slot(const int slot_index) const;
    void release_slot(const int slot_index);

    void render(const c_assets &assets, const cc::s_vec_2d_int window_size) const;

private:
    static constexpr int k_slot_cnt = 1024;
    static constexpr int k_available_slot_stack_size_max = 32;
    static constexpr int k_tex_unit_limit = 32; // TODO: Have the real texture unit limit have an effect.

    u_gl_id m_vert_array_gl_id = 0;
    u_gl_id m_vert_buf_gl_id = 0;
    u_gl_id m_elem_buf_gl_id = 0;

    c_bitset<k_slot_cnt> m_slot_activity;

    // This is a stack where the indexes of released slots are pushed, so that when a new slot needs to be taken an available index can be taken from the top here.
    int m_available_slot_stack[k_available_slot_stack_size_max] = {};
    int m_available_slot_stack_size = 0;

    int m_slot_tex_units[k_slot_cnt] = {}; // What texture unit each slot is mapped to.

    // NOTE: Consider bundling the below into a struct.
    s_asset_id m_tex_unit_tex_ids[k_tex_unit_limit] = {}; // What texture ID (the actual texture asset) each unit maps to.
    int m_tex_unit_ref_cnts[k_tex_unit_limit] = {}; // How many slots are mapped to each unit.

    int find_tex_unit_to_use(const s_asset_id tex_id) const;
};

class c_sprite_batch_layer
{
public:
    void render(const c_assets &assets, const cc::s_vec_2d_int window_size) const;
    void take_any_available_slot(const s_asset_id tex_id, int &batch_index, int &slot_index);

    inline void write_to_slot(const int batch_index, const int slot_index, const s_sprite_batch_slot_write_data &write_data, const c_assets &assets) const
    {
        m_batches[batch_index].write_to_slot(slot_index, write_data, assets);
    }

    inline void clear_slot(const int batch_index, const int slot_index) const
    {
        m_batches[batch_index].clear_slot(slot_index);
    }

    inline void release_slot(const int batch_index, const int slot_index)
    {
        m_batches[batch_index].release_slot(slot_index);
    }

private:
    std::vector<c_sprite_batch> m_batches;
};

class c_renderer
{
public:
    void render(const c_assets &assets, const cc::s_vec_2d_int window_size);

    inline s_sprite_batch_slot_key take_any_available_sprite_batch_slot(const int layer_index, const s_asset_id tex_id)
    {
        s_sprite_batch_slot_key key;
        m_sprite_batch_layers[layer_index].take_any_available_slot(tex_id, key.batch_index, key.slot_index);
        key.layer_index = layer_index;
        return key;
    }

    inline void reset_sprite_batch_layers(const int new_cnt)
    {
        m_sprite_batch_layers.clear();
        m_sprite_batch_layers.resize(new_cnt);
    }

    inline void write_to_sprite_batch_slot(const s_sprite_batch_slot_key &key, const s_sprite_batch_slot_write_data &write_data, const c_assets &assets) const
    {
        m_sprite_batch_layers[key.layer_index].write_to_slot(key.batch_index, key.slot_index, write_data, assets);
    }

    inline void clear_sprite_batch_slot(const s_sprite_batch_slot_key &key) const
    {
        m_sprite_batch_layers[key.layer_index].clear_slot(key.batch_index, key.slot_index);
    }

    inline void release_sprite_batch_slot(const s_sprite_batch_slot_key &key)
    {
        m_sprite_batch_layers[key.layer_index].release_slot(key.batch_index, key.slot_index);
    }

private:
    std::vector<c_sprite_batch_layer> m_sprite_batch_layers;
};

constexpr int k_sprite_quad_shader_prog_vert_cnt = 14;
