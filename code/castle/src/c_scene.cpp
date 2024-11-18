#include <castle/c_scene.h>

#include <array>

#if 0
const s_scene_type_info &get_scene_type_info(const ec_scene_type type)
{
    static const std::array<s_scene_type_info, k_scene_type_cnt> scene_type_infos = []()
    {
        std::array<s_scene_type_info, k_scene_type_cnt> scene_type_infos;

        auto make_scene_type_info = [](const ec_scene_type type)
        {
            switch (type)
            {
                case ec_scene_type::title:
                    return s_scene_type_info {
                        {1},
                        0
                    };

                case ec_scene_type::world:
                    return s_scene_type_info {
                        {8, 1, 1},
                        2
                    };
            }

            assert(false);

            return s_scene_type_info {};
        };

        for (int i = 0; i < k_scene_type_cnt; ++i)
        {
            scene_type_infos[i] = make_scene_type_info(static_cast<ec_scene_type>(i));
        }

        return scene_type_infos;
    }();

    return scene_type_infos[static_cast<int>(type)];
}

c_scene *make_scene(const ec_scene_type type, const c_assets &assets)
{
    const s_scene_type_info &scene_type_info = get_scene_type_info(type);

    s_sprite_batch_collection sprite_batch_collection = make_sprite_batch_collection(scene_type_info.layer_batch_cnts, scene_type_info.screen_layers_begin_batch_index);

    switch (type)
    {
        case ec_scene_type::title:
            return new c_title_scene(sprite_batch_collection);

        case ec_scene_type::world:
            return new c_world_scene(sprite_batch_collection, assets);
    }

    assert(false);
    return nullptr;
}

c_title_scene::c_title_scene(s_sprite_batch_collection &sprite_batch_collection) : c_scene(sprite_batch_collection)
{
}

void c_title_scene::tick(const s_input_state_pair &input_state_pair, const c_assets &assets, const cc::s_vec_2d_i window_size, bool &change_scene, ec_scene_type &change_scene_type)
{
    if (is_key_pressed(ec_key_code::enter, input_state_pair))
    {
        change_scene = true;
        change_scene_type = ec_scene_type::world;
    }
}

c_world_scene::c_world_scene(s_sprite_batch_collection &sprite_batch_collection, const c_assets &assets)
    : c_scene(sprite_batch_collection), m_player_ent(make_player_ent(k_world_scene_middle, sprite_batch_collection, assets)), m_tilemap(sprite_batch_collection)
{
    for (int x = 0; x < k_tilemap_size; ++x)
    {
        m_tilemap.place_tile(x, 0);
        m_tilemap.place_tile(x, k_tilemap_size - 1);
    }

    for (int y = 0; y < k_tilemap_size; ++y)
    {
        m_tilemap.place_tile(0, y);
        m_tilemap.place_tile(k_tilemap_size - 1, y);
    }
}

void c_world_scene::tick(const s_input_state_pair &input_state_pair, const c_assets &assets, const cc::s_vec_2d_i window_size, bool &change_scene, ec_scene_type &change_scene_type)
{
    m_player_ent = player_ent_after_tick(m_player_ent, input_state_pair, m_tilemap, assets, m_cam, window_size);
    write_player_ent_render_data(m_player_ent, m_sprite_batch_collection, assets);

    m_tilemap.write_render_data(m_sprite_batch_collection, assets);

    m_cam.pos = m_player_ent.pos;
}

void c_scene::render(const c_assets &assets, const cc::s_vec_2d_i window_size) const
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    render_sprite_batches(m_sprite_batch_collection, m_cam, assets, window_size);
}
#endif
