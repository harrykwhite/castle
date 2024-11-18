#include <castle/c_tilemap.h>

c_tilemap::c_tilemap(s_sprite_batch_collection &sprite_batch_collection)
{
    for (int y = 0; y < k_tilemap_size; ++y)
    {
        for (int x = 0; x < k_tilemap_size; ++x)
        {
            m_sb_slot_keys[y][x] = take_any_sprite_batch_slot(static_cast<int>(ec_sprite_batch_layer::tiles), s_asset_id::make_core_tex_id(ec_core_tex::dirt_tile), sprite_batch_collection);
        }
    }
}

void c_tilemap::write_render_data(const s_sprite_batch_collection &sprite_batch_collection, const c_assets &assets)
{
    // NOTE: Might be more efficient to just move through the thing in terms of bytes, not bits. You can check whether a byte is clear before checking bits.
    if (m_tile_render_rewrite.is_empty())
    {
        return;
    }

    const cc::s_rect src_rect = {0, 0, k_tile_size, k_tile_size};

    for (int y = 0; y < k_tilemap_size; ++y)
    {
        for (int x = 0; x < k_tilemap_size; ++x)
        {
            if (!m_tile_render_rewrite.is_bit_active((y * k_tilemap_size) + x))
            {
                continue;
            }

            if (m_tile_activity.is_bit_active((y * k_tilemap_size) + x))
            {
                const cc::s_vec_2d pos = {static_cast<float>(x * k_tile_size), static_cast<float>(y * k_tile_size)};
                write_to_sprite_batch_slot(m_sb_slot_keys[y][x], sprite_batch_collection, assets, pos, src_rect);
            }
            else
            {
                clear_sprite_batch_slot(m_sb_slot_keys[y][x], sprite_batch_collection);
            }
        }
    }

    m_tile_render_rewrite.clear();
}
