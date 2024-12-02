// An animation is effectively a collection of source rectangles (i.e. image subsets) associated with a single texture ID.
// Modders will need to be able to create their own animation types.

#pragma once

#include <castle_common/cc_assets.h>
#include <castle_common/cc_math.h>
#include "c_assets.h"

using AnimationSrcRectBuilder = cc::Rect(*)(const int frameIndex);

enum CoreAnimationTypeIndex
{
    PLAYER_ENT_IDLE_CORE_ANIM,

    CORE_ANIM_TYPE_CNT
};

struct AnimationType
{
    AnimationSrcRectBuilder srcRectBuilder;
    int srcRectCnt;
    AssetID texID;
};

extern AnimationType *g_coreAnimTypes; // TEMP: Not sure how we're going to store these yet considering the modding system.

// An instance of an animation mapped to a specific type. For example, you might have one of these per enemy.
// It will be common to change the animation type in this (e.g. when switching from an idle animation to a walking animation).
struct AnimationInst
{
    int frameIndex;
    int frameTime; // The number of ticks since the last frame change.
    int frameInterval; // The number of ticks before moving to the next frame.

    int typeIndex;
};

void init_core_anim_types(cc::MemArena &permMemArena);
void anim_inst_tick(AnimationInst &animInst);

inline cc::Rect get_anim_src_rect(const AnimationInst &animInst)
{
    return g_coreAnimTypes[animInst.typeIndex].srcRectBuilder(animInst.frameIndex);
}
