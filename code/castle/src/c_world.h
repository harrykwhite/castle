#pragma once

#include <castle_common/cc_math.h>
#include "c_input.h"
#include "c_assets.h"
#include "c_rendering.h"
#include "c_animation.h"
#include "c_audio.h"

constexpr int gk_enemyEntLimit = 64;
constexpr int gk_hitboxLimit = 16;

enum WorldRenderLayer
{
    // Camera Layers
    WORLD_ENEMY_ENT_LAYER,
    WORLD_PLAYER_ENT_LAYER,
    WORLD_HITBOX_LAYER, // TEMP: Only for debugging.

    // Non-Camera Layers
    WORLD_CURSOR_LAYER,

    WORLD_LAYER_CNT
};

struct PlayerEntSword
{
    SpriteBatchSlotKey sbSlotKey;
    bool rotNeg;
    float rotOffs;
};

struct PlayerEnt
{
    SpriteBatchSlotKey sbSlotKey;
    
    AnimationInst animInst;
    
    cc::Vec2D pos;
    float rot;

    PlayerEntSword sword;
};

// NOTE: Just a temporary grouping. Transform data will likely need to be distinguished from other data later on.
struct EnemyEnt
{
    SpriteBatchSlotKey sbSlotKey;
    cc::Vec2D pos;
    float rot;
    cc::Vec2D vel;
    int hp;
};

struct Hitbox
{
    cc::RectFloat rect;
    cc::Vec2D force; // Generally used for knockback.
    SpriteBatchSlotKey sbSlotKey; // For debug display.
};

struct World
{
    Renderer renderer;
    Camera cam;

    PlayerEnt playerEnt;

    EnemyEnt enemyEnts[gk_enemyEntLimit];
    StaticBitset<gk_enemyEntLimit> enemyEntActivity;

    Hitbox hitboxes[gk_hitboxLimit];
    StaticBitset<gk_hitboxLimit> hitboxActivity;

    SpriteBatchSlotKey cursorSBSlotKey;
};

void init_world(World &world, MusicManager &musicManager, cc::MemArena &permMemArena, cc::MemArena &tempMemArena, const AssetGroupManager &assetGroupManager);
void clean_world(World &world);
void world_tick(World &world, SoundManager &soundManager, const InputManager &inputManager, const AssetGroupManager &assetGroupManager);

void init_player_ent(World &world, const AssetGroupManager &assetGroupManager);
void player_ent_tick(World &world, SoundManager &soundManager, const InputManager &inputManager, const AssetGroupManager &assetGroupManager);

int spawn_enemy_ent(World &world, const cc::Vec2D pos, const AssetGroupManager &assetGroupManager);
void enemy_ent_tick(World &world, const int entIndex, const AssetGroupManager &assetGroupManager);
void hurt_enemy_ent(World &world, const int entIndex, const int dmg, const cc::Vec2D force);
cc::RectFloat make_enemy_ent_collider(EnemyEnt &enemyEnt, const AssetGroupManager &assetGroupManager);

int add_hitbox(World &world, const cc::RectFloat rect, const cc::Vec2D force);
