#include "c_world.h"

constexpr AssetID ik_enemyEntTexID = make_core_asset_id(cc::ENEMY_ENT_TEX); // TEMP: This will depend on enemy type.

static void write_enemy_ent_render_data(World &world, const int entIndex, const AssetGroupManager &assetGroupManager)
{
    assert(is_bit_active(world.enemyEntActivity, entIndex));

    EnemyEnt &ent = world.enemyEnts[entIndex];
    const cc::Vec2DInt texSize = assetGroupManager.get_tex_size(ik_enemyEntTexID);

    const SpriteBatchSlotWriteData writeData = {
        .pos = ent.pos,
        .srcRect = {0, 0, texSize.x, texSize.y},
        .origin = {0.5f, 0.5f},
        .rot = ent.rot,
        .scale = {1.0f, 1.0f},
        .alpha = 1.0f
    };

    write_to_sprite_batch_slot(world.renderer, world.playerEnt.sbSlotKey, writeData, assetGroupManager);
}

int spawn_enemy_ent(World &world, const cc::Vec2D pos, const AssetGroupManager &assetGroupManager)
{
    const int entIndex = first_inactive_bit_index(world.enemyEntActivity);

    if (entIndex != -1)
    {
        activate_bit(world.enemyEntActivity, entIndex);

        EnemyEnt &ent = world.enemyEnts[entIndex];

        ent = {
            .sbSlotKey = take_any_sprite_batch_slot(world.renderer, WORLD_ENEMY_ENT_LAYER, ik_enemyEntTexID),
            .pos = pos,
            .hp = 3
        };
    }

    return entIndex;
}

void enemy_ent_tick(World &world, const int entIndex, const AssetGroupManager &assetGroupManager)
{
    assert(is_bit_active(world.enemyEntActivity, entIndex));

    EnemyEnt &ent = world.enemyEnts[entIndex];

    ent.pos += ent.vel;
    ent.vel *= 0.9f;

    // Write render data.
    {
        const cc::Vec2DInt texSize = assetGroupManager.get_tex_size(ik_enemyEntTexID);

        const SpriteBatchSlotWriteData writeData = {
            .pos = ent.pos,
            .srcRect = {0, 0, texSize.x, texSize.y},
            .origin = {0.5f, 0.5f},
            .rot = ent.rot,
            .scale = {1.0f, 1.0f},
            .alpha = 1.0f
        };

        write_to_sprite_batch_slot(world.renderer, ent.sbSlotKey, writeData, assetGroupManager);
    }
}

void damage_enemy_ent(World &world, const int entIndex, const int dmg)
{
    assert(is_bit_active(world.enemyEntActivity, entIndex));

    EnemyEnt &ent = world.enemyEnts[entIndex];

    ent.hp -= dmg;

    // Handle entity death.
    if (ent.hp <= 0)
    {
        release_sprite_batch_slot(world.renderer, ent.sbSlotKey);
        deactivate_bit(world.enemyEntActivity, entIndex);
    }
}

cc::RectFloat make_enemy_ent_collider(EnemyEnt &enemyEnt, const AssetGroupManager &assetGroupManager)
{
    const cc::Vec2DInt texSize = assetGroupManager.get_tex_size(ik_enemyEntTexID);

    return {
        enemyEnt.pos - (texSize / 2.0f),
        texSize
    };
}
