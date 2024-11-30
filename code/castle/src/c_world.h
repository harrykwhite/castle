#pragma once

#include <castle_common/cc_math.h>
#include "c_input.h"
#include "c_assets.h"
#include "c_rendering.h"
#include "c_player_ent.h"

enum WorldRenderLayer
{
    WORLD_PLAYER_ENT_LAYER,
    WORLD_CURSOR_LAYER,

    WORLD_LAYER_CNT
};

struct World
{
    Renderer renderer;
    Camera cam;

    PlayerEnt playerEnt;

    SpriteBatchSlotKey cursorSBSlotKey;
};

void init_world(World &world, const AssetGroupManager &assetGroupManager);
void clean_world(World &world);
void world_tick(World &world, const InputManager &inputManager, const AssetGroupManager &assetGroupManager);
