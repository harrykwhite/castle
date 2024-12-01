#include "c_world.h"

#include "c_game.h"

constexpr AssetID ik_playerEntTexID = make_vanilla_asset_id(cc::PLAYER_ENT_VANILLA_TEX);

static void write_player_ent_render_data(World &world, const AssetGroupManager &assetGroupManager)
{
    const cc::Vec2DInt texSize = assetGroupManager.get_tex_size(ik_playerEntTexID);

    const SpriteBatchSlotWriteData writeData = {
        .pos = world.playerEnt.pos,
        .srcRect = get_anim_src_rect(world.playerEnt.animInst),
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
    
    //SpriteBatchSlotKey testKey = take_any_sprite_batch_slot(world.renderer, WORLD_ENEMY_ENT_LAYER, make_vanilla_asset_id(cc::ENEMY_ENT_VANILLA_TEX));
    //write_to_sprite_batch_slot(world.renderer, testKey, {cc::Vec2D {0.0f, 0.0f}, {0, 0, 20, 20}, {0.5f, 0.5f}, 0.0f, {1.0f, 1.0f}, 1.0f}, assetGroupManager);

    world.playerEnt.animInst.frameInterval = 20;
}

void player_ent_tick(World &world, const InputManager &inputManager, const AssetGroupManager &assetGroupManager)
{
    const cc::Vec2D moveAxis = {
        static_cast<float>(inputManager.is_key_down(KEY_D)) - inputManager.is_key_down(KEY_A),
        static_cast<float>(inputManager.is_key_down(KEY_S)) - inputManager.is_key_down(KEY_W)
    };

    world.playerEnt.pos += moveAxis * gk_playerMoveSpd;

    world.playerEnt.rot = cc::calc_dir(world.playerEnt.pos, screen_to_camera_pos(inputManager.get_mouse_pos(), world.cam));

    write_player_ent_render_data(world, assetGroupManager);

    anim_inst_tick(world.playerEnt.animInst);
}
