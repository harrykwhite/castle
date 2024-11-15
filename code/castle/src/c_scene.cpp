#include <castle/c_scene.h>

void c_title_scene::on_tick(const c_input_manager &input_manager, const c_assets &assets, const cc::s_vec_2d_int window_size, bool &change_scene, ec_scene_type &change_scene_type)
{
    if (input_manager.is_key_pressed(ec_key_code::enter))
    {
        change_scene = true;
        change_scene_type = ec_scene_type::gameplay;
    }
}

c_gameplay_scene::c_gameplay_scene(const int sb_layer_cnt) : c_scene(sb_layer_cnt), m_player_ent(m_renderer), m_tilemap(m_renderer)
{
    m_cam.scale = 2.0f;

    // TEMP
    m_tilemap.place_tile(3, 3);
    m_tilemap.place_tile(4, 3);
    m_tilemap.place_tile(5, 3);
}

void c_gameplay_scene::on_tick(const c_input_manager &input_manager, const c_assets &assets, const cc::s_vec_2d_int window_size, bool &change_scene, ec_scene_type &change_scene_type)
{
    m_player_ent.proc_movement(input_manager, m_tilemap, assets);
    m_player_ent.update_rot(input_manager, m_cam, window_size);
    m_player_ent.rewrite_render_data(m_renderer, assets);

    m_tilemap.rewrite_render_data(m_renderer, assets);

    m_cam.pos = m_player_ent.get_pos();
}

std::unique_ptr<c_scene> make_scene(const ec_scene_type scene_type)
{
    switch (scene_type)
    {
        case ec_scene_type::title:
            return std::make_unique<c_title_scene>(1);

        case ec_scene_type::gameplay:
            return std::make_unique<c_gameplay_scene>(2);
    }

    return nullptr;
}
