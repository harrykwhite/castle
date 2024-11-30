#include "c_player_ent.h"

#include "c_world.h"

constexpr AssetID ik_playerEntTexID = make_vanilla_asset_id(cc::PLAYER_ENT_VANILLA_TEX);

static void write_player_ent_render_data(World &world, const AssetGroupManager &assetGroupManager)
{
    const cc::Vec2DInt texSize = assetGroupManager.get_tex_size(ik_playerEntTexID);

    const SpriteBatchSlotWriteData writeData = {
        .pos = world.playerEnt.pos,
        .srcRect = {0, 0, texSize.x, texSize.y}, // TEMP: Use some kind of animation system later.
        .origin = {0.5f, 0.5f},
        .rot = world.playerEnt.rot,
        .scale = {1.0f, 1.0f},
        .alpha = 1.0f
    };

    write_to_sprite_batch_slot(world.renderer, world.playerEnt.sbSlotKey, writeData, assetGroupManager);
}

void init_player_ent(World &world, const AssetGroupManager &assetGroupManager)
{
    world.playerEnt.sbSlotKey = take_any_sprite_batch_slot(world.renderer, WORLD_PLAYER_ENT_LAYER, make_vanilla_asset_id(cc::PLAYER_ENT_VANILLA_TEX));
}

void player_ent_tick(World &world, const InputManager &inputManager, const AssetGroupManager &assetGroupManager, const cc::Vec2DInt windowSize)
{
    const cc::Vec2D moveAxis = {
        inputManager.is_key_down(KEY_D) - inputManager.is_key_down(KEY_A),
        inputManager.is_key_down(KEY_S) - inputManager.is_key_down(KEY_W)
    };

    world.playerEnt.pos += moveAxis * gk_playerMoveSpd;

    world.playerEnt.rot = cc::calc_dir(world.playerEnt.pos, screen_to_camera_pos(inputManager.get_mouse_pos(), world.cam, windowSize));

    write_player_ent_render_data(world, assetGroupManager);
}
