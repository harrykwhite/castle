#include "c_world.h"

static RenderLayerInitInfo render_layer_factory(const int index)
{
    switch (index)
    {
        case WORLD_ENEMY_ENT_LAYER:
            return {
                .spriteBatchCnt = 1,
                .spriteBatchSlotCnt = gk_enemyEntLimit,
                .charBatchCnt = 0
            };

        case WORLD_PLAYER_ENT_LAYER:
            return {
                .spriteBatchCnt = 1,
                .spriteBatchSlotCnt = 2,
                .charBatchCnt = 0
            };

        case WORLD_HITBOX_LAYER:
            return {
                .spriteBatchCnt = 1,
                .spriteBatchSlotCnt = gk_hitboxLimit,
                .charBatchCnt = 0
            };

        case WORLD_CURSOR_LAYER:
            return {
                .spriteBatchCnt = 1,
                .spriteBatchSlotCnt = 1,
                .charBatchCnt = 0
            };

        default:
            assert(false && "The world render layer factory does not support the provided layer index!");
            return {};
    }
}

static void write_cursor_render_data(World &world, const InputManager &inputManager, const AssetGroupManager &assetGroupManager)
{
    const cc::Vec2DInt texSize = assetGroupManager.get_tex_size(make_core_asset_id(cc::CURSOR_TEX));

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

void init_world(World &world, MusicManager &musicManager, cc::MemArena &permMemArena, cc::MemArena &tempMemArena, const AssetGroupManager &assetGroupManager)
{
    init_renderer(world.renderer, permMemArena, WORLD_LAYER_CNT, WORLD_HITBOX_LAYER + 1, render_layer_factory);

    init_player_ent(world, assetGroupManager);

    spawn_enemy_ent(world, {64.0f, 0.0f}, assetGroupManager);
    spawn_enemy_ent(world, {80.0f, 40.0f}, assetGroupManager);

    for (int i = 0; i < gk_hitboxLimit; ++i)
    {
        world.hitboxSBSlotKeys[i] = take_any_sprite_batch_slot(world.renderer, WORLD_HITBOX_LAYER, make_core_asset_id(cc::PIXEL_TEX));
    }

    cc::RectFloat hitboxes[gk_hitboxLimit];
    SpriteBatchSlotKey hitboxSBSlotKeys[gk_hitboxLimit];
    StaticBitset<gk_hitboxLimit> hitboxActivity;

    world.cursorSBSlotKey = take_any_sprite_batch_slot(world.renderer, WORLD_CURSOR_LAYER, make_core_asset_id(cc::CURSOR_TEX));

    // Start combat music.
    const MusicSrcID combatMusicSrcID = musicManager.add_src(make_core_asset_id(cc::COMBAT_MUSIC), assetGroupManager);
    musicManager.play_src(tempMemArena, combatMusicSrcID, assetGroupManager);
}

void clean_world(World &world)
{
    clean_renderer(world.renderer);
}

void world_tick(World &world, SoundManager &soundManager, const InputManager &inputManager, const AssetGroupManager &assetGroupManager)
{
    // Reset hitboxes.
    for (int i = 0; i < gk_hitboxLimit; ++i)
    {
        if (!is_bit_active(world.hitboxActivity, i))
        {
            continue;
        }

        clear_sprite_batch_slot(world.renderer, world.hitboxSBSlotKeys[i]);
        deactivate_bit(world.hitboxActivity, i);
    }

    // Execute player tick.
    player_ent_tick(world, soundManager, inputManager, assetGroupManager);

    // Execute enemy ticks.
    for (int i = 0; i < gk_enemyEntLimit; ++i)
    {
        if (!is_bit_active(world.enemyEntActivity, i))
        {
            continue;
        }

        enemy_ent_tick(world, i, assetGroupManager);
    }

    // Update hitboxes.
    for (int i = 0; i < gk_hitboxLimit; ++i)
    {
        if (!is_bit_active(world.hitboxActivity, i))
        {
            continue;
        }

        const cc::RectFloat &hitbox = world.hitboxes[i];

        // Check all enemies for a collision.
        for (int j = 0; j < gk_enemyEntLimit; ++j)
        {
            if (!is_bit_active(world.enemyEntActivity, j))
            {
                continue;
            }

            EnemyEnt &enemyEnt = world.enemyEnts[j];
            const cc::RectFloat enemyEntCollider = make_enemy_ent_collider(enemyEnt, assetGroupManager);

            if (cc::do_rects_intersect(hitbox, enemyEntCollider))
            {
                damage_enemy_ent(world, j, 1);
            }
        }

        // Write hitbox render data.
        const SpriteBatchSlotWriteData writeData = {
            .pos = hitbox.pos,
            .srcRect = {0, 0, 1, 1},
            .origin = {},
            .rot = 0.0f,
            .scale = hitbox.size,
            .alpha = 1.0f
        };

        write_to_sprite_batch_slot(world.renderer, world.hitboxSBSlotKeys[i], writeData, assetGroupManager);
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

int add_hitbox(World &world, const cc::RectFloat hitbox)
{
    const int index = first_inactive_bit_index(world.hitboxActivity);

    if (index != -1)
    {
        world.hitboxes[index] = hitbox;
        activate_bit(world.hitboxActivity, index);
    }

    return index;
}
