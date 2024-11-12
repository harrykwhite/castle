#include <castle/c_tilemap.h>

c_tilemap::c_tilemap(c_sprite_batch_layer &sb_layer)
{
    for (int y = 0; y < k_tilemap_size; ++y)
    {
        for (int x = 0; x < k_tilemap_size; ++x)
        {
            m_sb_slot_keys[y][x] = sb_layer.take_any_available_slot(s_asset_id::make_core_tex_id((x & 1) ? ec_core_tex::dirt_tile : ec_core_tex::stone_tile));
        }
    }
}

void c_tilemap::rewrite_tiles(c_sprite_batch_layer &sb_layer, const c_assets &assets)
{
    // NOTE: Might be more efficient to just move through the thing in terms of bytes, not bits. You can check whether a byte is clear before checking bits.
    if (m_tile_render_rewrite.is_empty())
    {
        return;
    }

    s_sprite_batch_slot_write_data write_data = {};
    write_data.scale = {1.0f, 1.0f};
    write_data.origin = {0.0f, 0.0f};
    write_data.src_rect.width = static_cast<float>(k_tile_size);
    write_data.src_rect.height = static_cast<float>(k_tile_size);
    write_data.blend = s_color::make_white();

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
                write_data.pos = {static_cast<float>(x * k_tile_size), static_cast<float>(y * k_tile_size)};
                sb_layer.write_to_slot(m_sb_slot_keys[y][x], write_data, assets);
            }
            else
            {
                sb_layer.clear_slot(m_sb_slot_keys[y][x]);
            }
        }
    }

    m_tile_render_rewrite.clear();
}
