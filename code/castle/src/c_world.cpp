#include "c_world.h"

static RenderLayerCreateInfo render_layer_factory(const int index)
{
    switch (index)
    {
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
    world.cursorSBSlotKey = take_any_sprite_batch_slot(world.renderer, WORLD_CURSOR_LAYER, make_vanilla_asset_id(cc::CURSOR_VANILLA_TEX));
}

void clean_world(World &world)
{
    clean_renderer(world.renderer);
}

void world_tick(World &world, const InputManager &inputManager, const AssetGroupManager &assetGroupManager, const cc::Vec2DInt windowSize)
{
    player_ent_tick(world, inputManager, assetGroupManager, windowSize);

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
