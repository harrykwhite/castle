#include "c_world.h"

#include "c_game.h"

static constexpr AssetID ik_texID = make_vanilla_asset_id(cc::PLAYER_ENT_VANILLA_TEX);
static constexpr float ik_moveSpd = 2.0f;
static constexpr cc::Vec2D ik_swordHitboxSize = {26.0f, 26.0f};
static constexpr int ik_swordHitboxDist = 34;
static constexpr float ik_swordRotOffsLimit = cc::degs_to_rads(120.0f);
static constexpr float ik_swordRotOffsLerpFactor = 0.3f;

static void write_player_ent_render_data(World &world, const AssetGroupManager &assetGroupManager)
{
    {
        const cc::Vec2DInt texSize = assetGroupManager.get_tex_size(ik_texID);

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

    {
        const cc::Vec2DInt texSize = assetGroupManager.get_tex_size(make_vanilla_asset_id(cc::SWORD_VANILLA_TEX));

        const SpriteBatchSlotWriteData writeData = {
            .pos = world.playerEnt.pos,
            .srcRect = {0, 0, texSize},
            .origin = {-0.25f, 0.5f},
            .rot = world.playerEnt.rot + world.playerEnt.sword.rotOffs,
            .scale = {1.0f, 1.0f},
            .alpha = 1.0f
        };

        write_to_sprite_batch_slot(world.renderer, world.playerEnt.sword.sbSlotKey, writeData, assetGroupManager);
    }
}

static float calc_sword_rot_offs_targ(const PlayerEntSword &sword)
{
    return ik_swordRotOffsLimit * (sword.rotNeg ? -1.0f : 1.0f);
}

void init_player_ent(World &world, const AssetGroupManager &assetGroupManager)
{
    world.playerEnt.sbSlotKey = take_any_sprite_batch_slot(world.renderer, WORLD_PLAYER_ENT_LAYER, ik_texID);
    world.playerEnt.animInst.frameInterval = 20;
    world.playerEnt.sword.sbSlotKey = take_any_sprite_batch_slot(world.renderer, WORLD_PLAYER_ENT_LAYER, make_vanilla_asset_id(cc::SWORD_VANILLA_TEX));
    world.playerEnt.sword.rotOffs = calc_sword_rot_offs_targ(world.playerEnt.sword);
}

void player_ent_tick(World &world, const InputManager &inputManager, const AssetGroupManager &assetGroupManager)
{
    PlayerEnt &ent = world.playerEnt;

    const cc::Vec2D moveAxis = {
        static_cast<float>(inputManager.is_key_down(KEY_D)) - inputManager.is_key_down(KEY_A),
        static_cast<float>(inputManager.is_key_down(KEY_S)) - inputManager.is_key_down(KEY_W)
    };

    ent.pos += moveAxis * ik_moveSpd;

    ent.rot = cc::calc_dir(ent.pos, screen_to_camera_pos(inputManager.get_mouse_pos(), world.cam));

    write_player_ent_render_data(world, assetGroupManager);

    anim_inst_tick(ent.animInst);

    if (inputManager.is_mouse_button_pressed(MOUSE_BUTTON_LEFT))
    {
        ent.sword.rotNeg = !ent.sword.rotNeg;

        const cc::Vec2D hitboxCenterPos = ent.pos + cc::make_dir_vec_2d(ent.rot, ik_swordHitboxDist);

        const cc::RectFloat hitbox = {
            hitboxCenterPos - (ik_swordHitboxSize / 2.0f),
            ik_swordHitboxSize
        };

        add_hitbox(world, hitbox);
    }

    ent.sword.rotOffs = cc::lerp(ent.sword.rotOffs, calc_sword_rot_offs_targ(ent.sword), ik_swordRotOffsLerpFactor);
}
