#pragma once

#include <fstream>
#include <castle_common/cc_math.h>
#include <castle_common/cc_debugging.h>
#include "c_rendering.h"

constexpr int k_tile_size = 16;
constexpr int k_tilemap_size = 32;

class c_tilemap
{
public:
    c_tilemap(c_sprite_batch_layer &sb_layer);

    void rewrite_tiles(c_sprite_batch_layer &sb_layer, const c_assets &assets);

#if 0
    void fetch_from_ifs(std::ifstream &ifs);
    void write_to_ofs(std::ofstream &ofs) const;
#endif

    inline void place_tile(const int x, const int y)
    {
        CC_CHECK(x >= 0 && y >= 0 && x < k_tilemap_size && y < k_tilemap_size);
        m_tile_activity.activate_bit((y * k_tilemap_size) + x);
        m_tile_render_rewrite.activate_bit((y * k_tilemap_size) + x);
    }

    inline void remove_tile(const int x, const int y)
    {
        CC_CHECK(x >= 0 && y >= 0 && x < k_tilemap_size && y < k_tilemap_size);
        m_tile_activity.deactivate_bit((y * k_tilemap_size) + x);
        m_tile_render_rewrite.activate_bit((y * k_tilemap_size) + x);
    }

    inline bool is_tile_active(const int x, const int y) const
    {
        CC_CHECK(x >= 0 && y >= 0 && x < k_tilemap_size && y < k_tilemap_size);
        return m_tile_activity.is_bit_active((y * k_tilemap_size) + x);
    }

    inline cc::s_rect_float get_tile_collider(const int x, const int y) const
    {
        CC_CHECK(x >= 0 && y >= 0 && x < k_tilemap_size && y < k_tilemap_size);
        CC_CHECK(is_tile_active(x, y));
        return {static_cast<float>(x * k_tile_size), static_cast<float>(y * k_tile_size), k_tile_size, k_tile_size};
    }

private:
    c_bitset<k_tilemap_size * k_tilemap_size> m_tile_activity; // TEMP: Would be good to embed this information within the tile ID data once we have that.
    c_bitset<k_tilemap_size * k_tilemap_size> m_tile_render_rewrite; // TEMP: A range-based refresh might work better, or even building a list of tiles to refresh.
    s_sprite_batch_slot_key m_sb_slot_keys[k_tilemap_size][k_tilemap_size] = {}; // TEMP: There is a lot of redundancy here; a range-based slot key is likely to be used later on as a substitute.
};
