#include "c_world.h"

static RenderLayerInitInfo render_layer_factory(const int index)
{
    switch (index)
    {
        case WORLD_ENEMY_ENT_LAYER:
            return {
                .spriteBatchCnt = 1,
                .spriteBatchSlotCnt = 128,
                .charBatchCnt = 1
            };

        case WORLD_PLAYER_ENT_LAYER:
            return {
                .spriteBatchCnt = 1,
                .spriteBatchSlotCnt = 1,
                .charBatchCnt = 1
            };

        case WORLD_CURSOR_LAYER:
            return {
                .spriteBatchCnt = 1,
                .spriteBatchSlotCnt = 1,
                .charBatchCnt = 1
            };

        default:
            assert(false);
            return {};
    }
}

static void write_cursor_render_data(World &world, const InputManager &inputManager, const AssetGroupManager &assetGroupManager)
{
    const cc::Vec2DInt texSize = assetGroupManager.get_tex_size(make_vanilla_asset_id(cc::CURSOR_VANILLA_TEX));

    const SpriteBatchSlotWriteData writeData = {
        .pos = inputManager.get_mouse_pos(),
        .srcRect = {0, 0, texSize.x, texSize.y},
        .origin = {0.5f, 0.5f},
        .rot = 0.0f,
        .scale = {1.0f, 1.0f},
        .alpha = 1.0f
    };

    write_to_sprite_batch_slot(world.renderer, world.cursorSBSlotKey, writeData, assetGroupManager);
}

void init_world(World &world, const AssetGroupManager &assetGroupManager)
{
    init_renderer(world.renderer, WORLD_LAYER_CNT, WORLD_PLAYER_ENT_LAYER + 1, render_layer_factory);
    
    init_player_ent(world, assetGroupManager);
    
    spawn_enemy_ent(world, {64.0f, 0.0f}, assetGroupManager);
    spawn_enemy_ent(world, {80.0f, 40.0f}, assetGroupManager);

    world.cursorSBSlotKey = take_any_sprite_batch_slot(world.renderer, WORLD_CURSOR_LAYER, make_vanilla_asset_id(cc::CURSOR_VANILLA_TEX));
}

void clean_world(World &world)
{
    clean_renderer(world.renderer);
}

void world_tick(World &world, const InputManager &inputManager, const AssetGroupManager &assetGroupManager)
{
    player_ent_tick(world, inputManager, assetGroupManager);

    // Execute enemy ticks.
    for (int i = 0; i < gk_enemyEntLimit; ++i)
    {
        if (!is_bit_active(world.enemyEntActivity, i))
        {
            continue;
        }

        // NOTE: If all active entities have their render data being written in this tick, it might just be better to reserve a buffer
        // for all enemies and then send the vertex data in one call.
        enemy_ent_tick(world, i, assetGroupManager);
    }

    // Write cursor render data.
    {
        const SpriteBatchSlotWriteData writeData = {
            .pos = inputManager.get_mouse_pos(),
            .srcRect = {0, 0, 4, 4},
            .origin = {0.5f, 0.5f},
            .rot = 0.0f,
            .scale = {1.0f, 1.0f},
            .alpha = 1.0f
        };

        write_to_sprite_batch_slot(world.renderer, world.cursorSBSlotKey, writeData, assetGroupManager);
    }
}
