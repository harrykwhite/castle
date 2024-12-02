#pragma once

#include "cc_math.h"

namespace cc
{

const char *const gk_assetsFileName = "assets.dat";

constexpr Vec2DInt gk_texSizeLimit = {2048, 2048};
constexpr int gk_texChannelCnt = 4;

constexpr int gk_fontCharRangeBegin = 32;
constexpr int gk_fontCharRangeSize = 95;

using AudioSample = short;

enum VanillaTexIndex
{
    PIXEL_VANILLA_TEX,
    PLAYER_ENT_VANILLA_TEX,
    ENEMY_ENT_VANILLA_TEX,
    SWORD_VANILLA_TEX,
    DIRT_TILE_VANILLA_TEX,
    STONE_TILE_VANILLA_TEX,
    INV_SLOT_VANILLA_TEX,
    CURSOR_VANILLA_TEX,

    VANILLA_TEX_CNT
};

enum VanillaFontIndex
{
    EB_GARAMOND_18_VANILLA_FONT,
    EB_GARAMOND_24_VANILLA_FONT,
    EB_GARAMOND_36_VANILLA_FONT,
    EB_GARAMOND_72_VANILLA_FONT,

    VANILLA_FONT_CNT
};

enum VanillaSoundIndex
{
    BLOOP_VANILLA_SOUND,

    VANILLA_SOUND_CNT
};

enum VanillaMusicIndex
{
    BEAT_VANILLA_MUSIC,

    VANILLA_MUSIC_CNT
};

struct FontCharsDisplayInfo
{
    int horOffsets[gk_fontCharRangeSize];
    int verOffsets[gk_fontCharRangeSize];
    int horAdvances[gk_fontCharRangeSize];

    Rect srcRects[gk_fontCharRangeSize];

    int kernings[gk_fontCharRangeSize * gk_fontCharRangeSize];
};

struct FontDisplayInfo
{
    int lineHeight;
    FontCharsDisplayInfo chars;
    Vec2DInt texSize;
};

struct AudioInfo
{
    int channelCnt;
    int sampleCntPerChannel;
    unsigned int sampleRate;
};

}
