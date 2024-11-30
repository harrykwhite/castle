#include "c_animation.h"

#include "c_game.h"

AnimationType *g_vanillaAnimTypes;

static cc::Rect player_ent_anim_src_rect_builder(const int frameIndex)
{
    switch (frameIndex)
    {
        case 0: return {0, 0, 24, 24};
        case 1: return {24, 0, 24, 24};

        default:
            assert(false);
            return {};
    }
}

void init_vanilla_anim_types()
{
    g_vanillaAnimTypes = cc::push_to_mem_arena<AnimationType>(g_permMemArena, VANILLA_ANIM_TYPE_CNT);

    g_vanillaAnimTypes[PLAYER_ENT_IDLE_VANILLA_ANIM] = {
        .srcRectBuilder = player_ent_anim_src_rect_builder,
        .srcRectCnt = 2,
        .texID = make_vanilla_asset_id(cc::PLAYER_ENT_VANILLA_TEX)
    };
}

void anim_inst_tick(AnimationInst &animInst)
{
    if (animInst.frameTime < animInst.frameInterval)
    {
        ++animInst.frameTime;
    }
    else
    {
        ++animInst.frameIndex;
        animInst.frameIndex %= g_vanillaAnimTypes[animInst.typeIndex].srcRectCnt;

        animInst.frameTime = 0;
    }
}
