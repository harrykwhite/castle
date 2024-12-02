#pragma once

#include <castle_common/cc_math.h>
#include "c_utils.h"
#include "c_assets.h"
#include "c_camera.h"

constexpr int gk_texUnitLimitCap = 32;

constexpr int gk_spriteBatchSlotVertsSize = gk_spriteQuadShaderProgVertCnt * 4;
constexpr int gk_charBatchSlotVertsSize = gk_charQuadShaderProgVertCnt * 4;

using TexUnit = char;

extern TexUnit g_texUnitLimit;

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
    int refCnt; // The number of slots in the batch using this texture unit.
};

struct SpriteBatch
{
    QuadBufGLIDs quadBufGLIDs;
    float *quadBufVerts; // The vertex data of the batch. The modified range of this buffer is submitted at the end of each frame in a single call.

    cc::Byte *slotActivity;
    TexUnit *slotTexUnits;
    cc::Range modifiedSlotRange;

    SpriteBatchTexUnitInfo *texUnitInfos;
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

struct CharBatch
{
    QuadBufGLIDs quadBufGLIDs;

    int slotCnt;

    AssetID fontID;

    cc::Vec2D pos;
    float rot;
    Color blend;
};

struct CharBatchKey
{
    int layerIndex;
    int batchIndex;
};

// A render layer is fundamentally a set of sprite batches and character batches.
// The implication of drawing things on the same layer is that you don't care about the order in which those things are drawn.
// Note however that the character batches in a layer are always drawn after (and therefore in front of) the sprite batches.
struct RenderLayer
{
    SpriteBatch *spriteBatches;
    int spriteBatchCnt;
    int spriteBatchSlotCnt; // All sprite batches in the same layer have the same slot count.
    cc::Byte *spriteBatchActivity;

    CharBatch *charBatches;
    int charBatchCnt;
    cc::Byte *charBatchActivity;
};

struct RenderLayerInitInfo
{
    int spriteBatchCnt;
    int spriteBatchSlotCnt;
    int charBatchCnt;
};

using RenderLayerInitInfoFactory = RenderLayerInitInfo (*)(const int index);

struct Renderer
{
    int layerCnt;
    int camLayerCnt; // Layers 0 through to this number exclusive are drawn with a camera view matrix.

    RenderLayer *layers;
};

void init_tex_unit_limit();

QuadBufGLIDs make_quad_buf(const int quadCnt, const bool isSprite);
void clean_quad_buf(QuadBufGLIDs &glIDs);

void init_renderer(Renderer &renderer, const int layerCnt, const int camLayerCnt, const RenderLayerInitInfoFactory layerInitInfoFactory);
void clean_renderer(Renderer &renderer);

void render(const Renderer &renderer, const Color &bgColor, const AssetGroupManager &assetGroupManager, const ShaderProgs &shaderProgs, const Camera *const cam);

SpriteBatchSlotKey take_any_sprite_batch_slot(Renderer &renderer, const int layerIndex, const AssetID texID);
void release_sprite_batch_slot(Renderer &renderer, const SpriteBatchSlotKey &key);
void write_to_sprite_batch_slot(Renderer &renderer, const SpriteBatchSlotKey &key, const SpriteBatchSlotWriteData &writeData, const AssetGroupManager &assetGroupManager);
void clear_sprite_batch_slot(const Renderer &renderer, const SpriteBatchSlotKey &key);
void submit_sprite_batch_slots(Renderer &renderer);

CharBatchKey activate_any_char_batch(Renderer &renderer, const int layerIndex, const int slotCnt, const AssetID fontID, const cc::Vec2D pos, const AssetGroupManager &assetGroupManager);
void deactivate_char_batch(Renderer &renderer, const CharBatchKey &key);
void write_to_char_batch(Renderer &renderer, const CharBatchKey &key, const char *const text, const FontHorAlign horAlign, const FontVerAlign verAlign, const AssetGroupManager &assetGroupManager);
void clear_char_batch(const Renderer &renderer, const CharBatchKey &key);
