#include "c_world.h"

#include "c_game.h"

static constexpr AssetID ik_texID = make_core_asset_id(cc::PLAYER_ENT_TEX);
static constexpr float ik_moveSpd = 2.0f;
static constexpr cc::Vec2D ik_swordHitboxSize = {26.0f, 26.0f};
static constexpr int ik_swordHitboxDist = 34;
static constexpr float ik_swordHitboxStrength = 11.0f;
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
        const cc::Vec2DInt texSize = assetGroupManager.get_tex_size(make_core_asset_id(cc::SWORD_TEX));

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
    world.playerEnt.sword.sbSlotKey = take_any_sprite_batch_slot(world.renderer, WORLD_PLAYER_ENT_LAYER, make_core_asset_id(cc::SWORD_TEX));
    world.playerEnt.sword.rotOffs = calc_sword_rot_offs_targ(world.playerEnt.sword);
}

void player_ent_tick(World &world, SoundManager &soundManager, const InputManager &inputManager, const AssetGroupManager &assetGroupManager)
{
    PlayerEnt &ent = world.playerEnt;

    //
    // Movement
    //
    const cc::Vec2D moveAxis = {
        static_cast<float>(inputManager.is_key_down(KEY_D)) - inputManager.is_key_down(KEY_A),
        static_cast<float>(inputManager.is_key_down(KEY_S)) - inputManager.is_key_down(KEY_W)
    };

    const cc::Vec2D velTarg = moveAxis * ik_moveSpd;
    ent.vel = cc::lerp(ent.vel, velTarg, 0.3f);
    ent.pos += ent.vel;

    ent.rot = cc::calc_dir(ent.pos, screen_to_camera_pos(inputManager.get_mouse_pos(), world.cam));

    //
    // Enemy Collisions
    //
    {
        const cc::RectFloat &collider = make_player_ent_collider(ent, assetGroupManager);

        for (int i = 0; i < gk_enemyEntLimit; ++i)
        {
            if (!is_bit_active(world.enemyEntActivity, i))
            {
                continue;
            }

            EnemyEnt &enemyEnt = world.enemyEnts[i];
            const cc::RectFloat enemyEntCollider = make_enemy_ent_collider(enemyEnt, assetGroupManager);
        
            if (cc::do_rects_intersect(collider, enemyEntCollider))
            {
                ent.vel = (ent.pos - enemyEnt.pos).normalized() * 13.0f;
                break;
            }
        }
    }

    //
    // Sword
    //
    if (inputManager.is_mouse_button_pressed(MOUSE_BUTTON_LEFT))
    {
        ent.sword.rotNeg = !ent.sword.rotNeg;

        soundManager.add_and_play_src(make_core_asset_id(cc::SWING_SOUND), assetGroupManager);

        const cc::Vec2D forwards = cc::make_dir_vec_2d(ent.rot);

        const cc::Vec2D hitboxRectCenterPos = ent.pos + (forwards * ik_swordHitboxDist);

        const cc::RectFloat hitboxRect = {
            hitboxRectCenterPos - (ik_swordHitboxSize / 2.0f),
            ik_swordHitboxSize
        };

        add_hitbox(world, hitboxRect, forwards * ik_swordHitboxStrength);
    }

    ent.sword.rotOffs = cc::lerp(ent.sword.rotOffs, calc_sword_rot_offs_targ(ent.sword), ik_swordRotOffsLerpFactor);

    //
    // Display
    //
    write_player_ent_render_data(world, assetGroupManager);
    anim_inst_tick(ent.animInst);
}

cc::RectFloat make_player_ent_collider(PlayerEnt &ent, const AssetGroupManager &assetGroupManager)
{
    const cc::Vec2DInt size = get_anim_src_rect(ent.animInst).size;

    return {
        ent.pos - (size / 2.0f),
        size
    };
}
