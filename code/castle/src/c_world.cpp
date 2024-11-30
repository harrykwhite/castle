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

void init_world(World &world, const AssetGroupManager &assetGroupManager)
{
    init_renderer(world.renderer, WORLD_LAYER_CNT, WORLD_PLAYER_ENT_LAYER + 1, render_layer_factory);
    init_player_ent(world, assetGroupManager);
}

void clean_world(World &world)
{
    clean_renderer(world.renderer);
}

void world_tick(World &world, const InputManager &inputManager, const AssetGroupManager &assetGroupManager, const cc::Vec2DInt windowSize)
{
    player_ent_tick(world, inputManager, assetGroupManager, windowSize);
}
