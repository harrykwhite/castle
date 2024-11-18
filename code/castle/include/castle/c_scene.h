#pragma once

#if 0
#include <vector>
#include "c_tilemap.h"
#include "c_rendering.h"
#include "c_input.h"
#include "c_player.h"

constexpr cc::s_vec_2d k_world_scene_middle = cc::s_vec_2d {k_tile_size, k_tile_size} * static_cast<float>(k_tilemap_size) * 0.5f;

enum class ec_scene_type
{
    title,
    world,

    LAST
};

constexpr int k_scene_type_cnt = static_cast<int>(ec_scene_type::LAST);

struct s_scene_type_info
{
    std::vector<int> layer_batch_cnts;
    int screen_layers_begin_batch_index;

    s_scene_type_info() : screen_layers_begin_batch_index(0)
    {
    }

    s_scene_type_info(const std::vector<int> layer_batch_cnts, const int screen_layers_begin_batch_index) : layer_batch_cnts(layer_batch_cnts), screen_layers_begin_batch_index(screen_layers_begin_batch_index)
    {
    }
};

class c_scene
{
public:
    c_scene(const s_sprite_batch_collection &sprite_batch_collection) : m_sprite_batch_collection(sprite_batch_collection)
    {
    }

    virtual void tick(const s_input_state_pair &input_state_pair, const c_assets &assets, const cc::s_vec_2d_i window_size, bool &change_scene, ec_scene_type &change_scene_type) = 0;
    void render(const c_assets &assets, const cc::s_vec_2d_i window_size) const;

protected:
    s_camera m_cam = {};
    s_sprite_batch_collection m_sprite_batch_collection;
};

class c_title_scene : public c_scene
{
public:
    c_title_scene(s_sprite_batch_collection &sprite_batch_collection);
    virtual void tick(const s_input_state_pair &input_state_pair, const c_assets &assets, const cc::s_vec_2d_i window_size, bool &change_scene, ec_scene_type &change_scene_type) override;

private:
};

class c_world_scene : public c_scene
{
public:
    c_world_scene(s_sprite_batch_collection &sprite_batch_collection, const c_assets &assets);
    virtual void tick(const s_input_state_pair &input_state_pair, const c_assets &assets, const cc::s_vec_2d_i window_size, bool &change_scene, ec_scene_type &change_scene_type) override;

private:
    s_player_ent m_player_ent;
    c_tilemap m_tilemap;
};

const s_scene_type_info &get_scene_type_info(const ec_scene_type type);
c_scene *make_scene(const ec_scene_type type, const c_assets &assets);
#endif
