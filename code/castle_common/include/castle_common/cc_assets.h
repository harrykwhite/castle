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

enum CoreTexIndex
{
    PIXEL_TEX,
    PLAYER_ENT_TEX,
    ENEMY_ENT_TEX,
    SWORD_TEX,
    DIRT_TILE_TEX,
    STONE_TILE_TEX,
    INV_SLOT_TEX,
    CURSOR_TEX,

    CORE_TEX_CNT
};

enum CoreFontIndex
{
    EB_GARAMOND_18_FONT,
    EB_GARAMOND_24_FONT,
    EB_GARAMOND_36_FONT,
    EB_GARAMOND_72_FONT,

    CORE_FONT_CNT
};

enum CoreSoundIndex
{
    BLOOP_SOUND,

    CORE_SOUND_CNT
};

enum CoreMusicIndex
{
    BEAT_MUSIC,

    CORE_MUSIC_CNT
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
