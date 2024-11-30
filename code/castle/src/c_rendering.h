#pragma once

#include <castle_common/cc_math.h>
#include "c_utils.h"
#include "c_assets.h"
#include "c_camera.h"

constexpr int gk_texUnitLimitCap = 32;

constexpr int gk_renderLayerLimit = 16;
constexpr int gk_renderLayerSpriteBatchLimit = 10;
constexpr int gk_renderLayerCharBatchLimit = 24;
constexpr int gk_spriteBatchSlotLimit = 1024;
constexpr int gk_charBatchSlotLimit = 1024;

enum FontHorAlign
{
    FONT_HOR_ALIGN_LEFT,
    FONT_HOR_ALIGN_CENTER,
    FONT_HOR_ALIGN_RIGHT
};

enum FontVerAlign
{
    FONT_VER_ALIGN_TOP,
    FONT_VER_ALIGN_CENTER,
    FONT_VER_ALIGN_BOTTOM
};

struct Color
{
    float r, g, b, a;

    static constexpr Color make_white() { return {1.0f, 1.0f, 1.0f, 1.0f}; }
    static constexpr Color make_black() { return {0.0f, 0.0f, 0.0f, 1.0f}; }
    static constexpr Color make_red() { return {1.0f, 0.0f, 0.0f, 1.0f}; }
    static constexpr Color make_green() { return {0.0f, 1.0f, 0.0f, 1.0f}; }
    static constexpr Color make_blue() { return {0.0f, 0.0f, 1.0f, 1.0f}; }
    static constexpr Color make_yellow() { return {1.0f, 1.0f, 0.0f, 1.0f}; }
    static constexpr Color make_cyan() { return {0.0f, 1.0f, 1.0f, 1.0f}; }
    static constexpr Color make_magenta() { return {1.0f, 0.0f, 1.0f, 1.0f}; }
};

struct QuadBufGLIDs
{
    GLID vertArrayGLID;
    GLID vertBufGLID;
    GLID elemBufGLID;
};

struct SpriteBatchTexUnitInfo
{
    AssetID texID;
    int refCnt;
};

struct SpriteBatchSlotKey
{
    int layerIndex;
    int batchIndex;
    int slotIndex;
};

struct SpriteBatchSlotWriteData
{
    cc::Vec2D pos;
    cc::Rect srcRect;
    cc::Vec2D origin;
    float rot;
    cc::Vec2D scale;
    float alpha;

    static inline SpriteBatchSlotWriteData make(const cc::Vec2D pos, const cc::Rect &srcRect)
    {
        return {
            .pos = pos,
            .srcRect = srcRect,
            .origin = {0.5f, 0.5f},
            .rot = 0.0f,
            .scale = {1.0f, 1.0f},
            .alpha = 1.0f
        };
    }
};

struct SpriteBatches
{
    QuadBufGLIDs *quadBufGLIDs;

    int *slotTexUnits;
    SpriteBatchTexUnitInfo *texUnitInfos;

    HeapBitset slotActivity[gk_renderLayerSpriteBatchLimit];
};

struct CharBatchDisplayProps
{
    cc::Vec2D pos;
    float rot;
    Color blend;
};

struct CharBatchKey
{
    int layerIndex;
    int batchIndex;
};

struct CharBatches
{
    int *slotCnts;
    QuadBufGLIDs *quadBufGLIDs;
    AssetID *fontIDs;
    CharBatchDisplayProps *displayProps;
};

// A render layer is fundamentally a set of sprite batches and character batches.
// The implication of drawing many things on the same layer is that you don't care about the order in which those things are drawn.
struct RenderLayer
{
    int spriteBatchCnt;
    int spriteBatchSlotCnt; // All sprite batches in the same layer have the same slot count.

    int charBatchCnt;

    SpriteBatches spriteBatches;
    CharBatches charBatches;

    HeapBitset spriteBatchActivity;
    HeapBitset charBatchActivity;

    inline SpriteBatchTexUnitInfo get_sprite_batch_tex_unit_info(const int batchIndex, const int texUnit) const;

    inline int get_sprite_batch_slot_tex_unit(const int batchIndex, const int slotIndex) const
    {
        return spriteBatches.slotTexUnits[(batchIndex * spriteBatchSlotCnt) + slotIndex];
    }
};

struct RenderLayerCreateInfo
{
    int spriteBatchCnt;
    int spriteBatchSlotCnt;
    int charBatchCnt;
};

using RenderLayerFactory = RenderLayerCreateInfo(*)(const int index);

struct Renderer
{
    int layerCnt;
    int camLayerCnt; // Layers 0 through to this number exclusive are drawn with a camera view matrix.

    RenderLayer layers[gk_renderLayerLimit];
};

QuadBufGLIDs make_quad_buf(const int quadCnt, const bool isSprite);
void clean_quad_buf(QuadBufGLIDs &glIDs);

RenderLayer make_render_layer(const RenderLayerCreateInfo &createInfo);
void clean_render_layer(RenderLayer &layer);

void init_renderer(Renderer &renderer, const int layerCnt, const int camLayerCnt, const RenderLayerFactory layerFactory);
void clean_renderer(Renderer &renderer);

void draw_render_layers(const Renderer &renderer, const Color &bgColor, const AssetGroupManager &assetGroupManager, const ShaderProgGLIDs &shaderProgGLIDs, const Camera *const cam);

RenderLayer make_render_layer(const RenderLayerCreateInfo &createInfo);
void clean_render_layer(RenderLayer &layer);

SpriteBatchSlotKey take_any_sprite_batch_slot(Renderer &renderer, const int layerIndex, const AssetID texID);
void release_sprite_batch_slot(Renderer &renderer, const SpriteBatchSlotKey &key);
void write_to_sprite_batch_slot(const Renderer &renderer, const SpriteBatchSlotKey &key, const SpriteBatchSlotWriteData &writeData, const AssetGroupManager &assetGroupManager);
void clear_sprite_batch_slot(const Renderer &renderer, const SpriteBatchSlotKey &key);

CharBatchKey activate_any_char_batch(Renderer &renderer, const int layerIndex, const int slotCnt, const AssetID fontID, const AssetGroupManager &assetGroupManager);
void deactivate_char_batch(Renderer &renderer, const CharBatchKey &key);
void write_to_char_batch(Renderer &renderer, const CharBatchKey &key, const char *const text, const FontHorAlign horAlign, const FontVerAlign verAlign, const AssetGroupManager &assetGroupManager);
void clear_char_batch(const Renderer &renderer, const CharBatchKey &key);

inline void set_char_batch_pos(Renderer &renderer, const CharBatchKey &key, const cc::Vec2D pos)
{
    renderer.layers[key.layerIndex].charBatches.displayProps[key.batchIndex].pos = pos;
}

inline void set_char_batch_rot(Renderer &renderer, const CharBatchKey &key, const float rot)
{
    renderer.layers[key.layerIndex].charBatches.displayProps[key.batchIndex].rot = rot;
}

inline void set_char_batch_blend(Renderer &renderer, const CharBatchKey &key, const Color &blend)
{
    renderer.layers[key.layerIndex].charBatches.displayProps[key.batchIndex].blend = blend;
}
