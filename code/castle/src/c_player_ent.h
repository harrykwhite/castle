#pragma once

#include <castle_common/cc_math.h>
#include "c_input.h"
#include "c_assets.h"
#include "c_rendering.h"
#include "c_animation.h"

constexpr float gk_playerMoveSpd = 2.0f;

struct World;

struct PlayerEnt
{
    SpriteBatchSlotKey sbSlotKey;
    AnimationInst animInst;
    cc::Vec2D pos;
    float rot;
};

void init_player_ent(World &world, const AssetGroupManager &assetGroupManager);
void player_ent_tick(World &world, const InputManager &inputManager, const AssetGroupManager &assetGroupManager, const cc::Vec2DInt windowSize);
