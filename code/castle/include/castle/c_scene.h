#pragma once

#include <functional>
#include "c_input.h"
#include "c_rendering.h"
#include "c_player.h"
#include "c_tilemap.h"

enum class ec_scene_type
{
    title,
    gameplay
};

constexpr int k_scene_type_cnt = static_cast<int>(ec_scene_type::gameplay) + 1;

class c_scene
{
public:
    c_scene(const int sb_layer_cnt) : m_renderer(sb_layer_cnt) {}
    ~c_scene() = default;

    virtual void on_tick(const c_input_manager &input_manager, const c_assets &assets, const cc::s_vec_2d_int window_size, bool &change_scene, ec_scene_type &change_scene_type) = 0;

    inline void render(const c_assets &assets, const cc::s_vec_2d_int window_size)
    {
        m_renderer.render(m_cam, assets, window_size);
    }

protected:
    s_camera m_cam = {{}, 1.0f};
    c_renderer m_renderer;
};

class c_title_scene : public c_scene
{
public:
    c_title_scene(const int sb_layer_cnt) : c_scene(sb_layer_cnt) {}
    ~c_title_scene() = default;

    virtual void on_tick(const c_input_manager &input_manager, const c_assets &assets, const cc::s_vec_2d_int window_size, bool &change_scene, ec_scene_type &change_scene_type) override;

private:
};

class c_gameplay_scene : public c_scene
{
public:
    c_gameplay_scene(const int sb_layer_cnt);
    ~c_gameplay_scene() = default;

    virtual void on_tick(const c_input_manager &input_manager, const c_assets &assets, const cc::s_vec_2d_int window_size, bool &change_scene, ec_scene_type &change_scene_type) override;

private:
    c_player_ent m_player_ent;
    c_tilemap m_tilemap;
};

std::unique_ptr<c_scene> make_scene(const ec_scene_type scene_type);
