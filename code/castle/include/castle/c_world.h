#pragma once

#include "c_tilemap.h"
#include "c_rendering.h"
#include "c_player.h"

struct s_world
{
    c_tilemap tilemap;
    s_player_ent player_ent;
    s_camera cam;
};

s_world make_world(s_sprite_batch_collection &sprite_batch_collection, const c_assets &assets)
{
    const cc::s_vec_2d player_ent_spawn_pos = (cc::s_vec_2d {k_tile_size, k_tile_size}) * static_cast<float>(k_tilemap_size) * 0.5f;

    c_tilemap tilemap(sprite_batch_collection);

    for (int x = 0; x < k_tilemap_size; ++x)
    {
        tilemap.place_tile(x, 0);
        tilemap.place_tile(x, k_tilemap_size - 1);
    }

    for (int y = 0; y < k_tilemap_size; ++y)
    {
        tilemap.place_tile(0, y);
        tilemap.place_tile(k_tilemap_size - 1, y);
    }

    return {
        tilemap,
        make_player_ent(player_ent_spawn_pos, sprite_batch_collection, assets)
    };
}

void world_tick(s_world &world, const s_input_state_pair &input_state_pair, s_sprite_batch_collection &sprite_batch_collection, const c_assets &assets, const cc::s_vec_2d_i window_size)
{
    world.player_ent = player_ent_after_tick(world.player_ent, input_state_pair, world.tilemap, assets, world.cam, window_size);
    write_player_ent_render_data(world.player_ent, sprite_batch_collection, assets);

    world.tilemap.write_render_data(sprite_batch_collection, assets);

    world.cam.pos = world.player_ent.pos;
}
