#include <castle/c_tilemap.h>

c_tilemap::c_tilemap(c_renderer &renderer)
{
    for (int y = 0; y < k_tilemap_size; ++y)
    {
        for (int x = 0; x < k_tilemap_size; ++x)
        {
            m_sb_slot_keys[y][x] = renderer.take_any_sprite_batch_slot(0, s_asset_id::create_core_tex_id(ec_core_tex::dirt_tile));
        }
    }
}

void c_tilemap::write_render_data(const c_renderer &renderer, const c_assets &assets)
{
    if (m_tile_render_rewrite.none())
    {
        return;
    }

    const cc::s_rect src_rect = {0, 0, k_tile_size, k_tile_size};

    for (int y = 0; y < k_tilemap_size; ++y)
    {
        for (int x = 0; x < k_tilemap_size; ++x)
        {
            if (!m_tile_render_rewrite.test((y * k_tilemap_size) + x))
            {
                continue;
            }

            if (m_tile_activity.test((y * k_tilemap_size) + x))
            {
                const cc::s_vec_2d pos = {static_cast<float>(x * k_tile_size), static_cast<float>(y * k_tile_size)};
                renderer.write_to_sprite_batch_slot(m_sb_slot_keys[y][x], assets, pos, src_rect);
            }
            else
            {
                renderer.clear_sprite_batch_slot(m_sb_slot_keys[y][x]);
            }
        }
    }

    m_tile_render_rewrite.reset();
}
