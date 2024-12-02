#include "c_animation.h"

#include "c_game.h"

AnimationType *g_coreAnimTypes;

static
cc::Rect player_ent_anim_src_rect_builder(const int frameIndex)
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

void init_core_anim_types(cc::MemArena &permMemArena)
{
    g_coreAnimTypes = cc::push_to_mem_arena<AnimationType>(permMemArena, CORE_ANIM_TYPE_CNT);

    g_coreAnimTypes[PLAYER_ENT_IDLE_CORE_ANIM] = {
        .srcRectBuilder = player_ent_anim_src_rect_builder,
        .srcRectCnt = 2,
        .texID = make_core_asset_id(cc::PLAYER_ENT_TEX)
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
        animInst.frameIndex %= g_coreAnimTypes[animInst.typeIndex].srcRectCnt;

        animInst.frameTime = 0;
    }
}
