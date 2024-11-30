#include "c_rendering.h"

#include <numeric>
#include "c_game.h"

static inline int get_tex_unit_limit()
{
    int limit;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &limit);
    return std::min(limit, gk_texUnitLimitCap);
}

static int find_sprite_batch_tex_unit_to_use(const RenderLayer &renderLayer, const int batchIndex, const AssetID texID)
{
    int freeTexUnit = -1;

    const int texUnitLimit = get_tex_unit_limit();

    for (int i = 0; i < texUnitLimit; ++i)
    {
        const SpriteBatchTexUnitInfo &texUnit = renderLayer.spriteBatches.texUnitInfos[(batchIndex * texUnitLimit) + i];

        if (!texUnit.refCnt)
        {
            if (freeTexUnit == -1)
            {
                freeTexUnit = i;
            }

            continue;
        }

        if (texUnit.texID == texID)
        {
            return i;
        }
    }

    return freeTexUnit;
}

static void activate_any_sprite_batch(Renderer &renderer, const int layerIndex)
{
    assert(layerIndex >= 0 && layerIndex < renderer.layerCnt);

    RenderLayer &layer = renderer.layers[layerIndex];

    const int batchIndex = first_inactive_bit_index(layer.spriteBatchActivity);
    assert(batchIndex != -1);

    activate_bit(layer.spriteBatchActivity, batchIndex);

    layer.spriteBatches.quadBufGLIDs[batchIndex] = make_quad_buf(layer.spriteBatchSlotCnt, true);
    memset(layer.spriteBatches.slotTexUnits + (batchIndex * layer.spriteBatchSlotCnt), 0, layer.spriteBatchSlotCnt * sizeof(int));
    memset(layer.spriteBatches.texUnitInfos + (batchIndex * get_tex_unit_limit()), 0, get_tex_unit_limit() * sizeof(SpriteBatchTexUnitInfo));
    clear_bits(layer.spriteBatches.slotActivity[batchIndex]);
}

inline SpriteBatchTexUnitInfo RenderLayer::get_sprite_batch_tex_unit_info(const int batchIndex, const int texUnit) const
{
    return spriteBatches.texUnitInfos[(batchIndex * get_tex_unit_limit()) + texUnit];
}

QuadBufGLIDs make_quad_buf(const int quadCnt, const bool isSprite)
{
    assert(quadCnt > 0);

    QuadBufGLIDs glIDs = {};

    const int vertCnt = isSprite ? gk_spriteQuadShaderProgVertCnt : gk_charQuadShaderProgVertCnt;

    // Generate vertex array.
    glGenVertexArrays(1, &glIDs.vertArrayGLID);
    glBindVertexArray(glIDs.vertArrayGLID);

    // Generate vertex buffer.
    glGenBuffers(1, &glIDs.vertBufGLID);
    glBindBuffer(GL_ARRAY_BUFFER, glIDs.vertBufGLID);

    {
        const int vertsLen = vertCnt * 4 * quadCnt;
        const auto verts = cc::push_to_mem_arena<float>(g_tempMemArena, vertsLen);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts[0]) * vertsLen, verts, GL_DYNAMIC_DRAW);
    }

    // Generate element buffer.
    glGenBuffers(1, &glIDs.elemBufGLID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glIDs.elemBufGLID);

    {
        // TODO: Have it so that the indices are only set up once; all batches use a copy of this same data, so recalculation isn't needed.
        const int indicesLen = 6 * quadCnt;
        const auto indices = cc::push_to_mem_arena<unsigned short>(g_tempMemArena, indicesLen);

        for (int i = 0; i < quadCnt; i++)
        {
            indices[(i * 6) + 0] = (i * 4) + 0;
            indices[(i * 6) + 1] = (i * 4) + 1;
            indices[(i * 6) + 2] = (i * 4) + 2;
            indices[(i * 6) + 3] = (i * 4) + 2;
            indices[(i * 6) + 4] = (i * 4) + 3;
            indices[(i * 6) + 5] = (i * 4) + 0;
        }

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indicesLen, indices, GL_STATIC_DRAW);
    }

    // Set vertex attribute pointers.
    const int vertsStride = sizeof(float) * vertCnt;

    if (isSprite)
    {
        glVertexAttribPointer(0, 2, GL_FLOAT, false, vertsStride, reinterpret_cast<void *>(sizeof(float) * 0));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, false, vertsStride, reinterpret_cast<void *>(sizeof(float) * 2));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, false, vertsStride, reinterpret_cast<void *>(sizeof(float) * 4));
        glEnableVertexAttribArray(2);

        glVertexAttribPointer(3, 1, GL_FLOAT, false, vertsStride, reinterpret_cast<void *>(sizeof(float) * 6));
        glEnableVertexAttribArray(3);

        glVertexAttribPointer(4, 1, GL_FLOAT, false, vertsStride, reinterpret_cast<void *>(sizeof(float) * 7));
        glEnableVertexAttribArray(4);

        glVertexAttribPointer(5, 2, GL_FLOAT, false, vertsStride, reinterpret_cast<void *>(sizeof(float) * 8));
        glEnableVertexAttribArray(5);

        glVertexAttribPointer(6, 1, GL_FLOAT, false, vertsStride, reinterpret_cast<void *>(sizeof(float) * 10));
        glEnableVertexAttribArray(6);
    }
    else
    {
        glVertexAttribPointer(0, 2, GL_FLOAT, false, vertsStride, reinterpret_cast<void *>(sizeof(float) * 0));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, false, vertsStride, reinterpret_cast<void *>(sizeof(float) * 2));
        glEnableVertexAttribArray(1);
    }

    // Unbind.
    glBindVertexArray(0);

    return glIDs;
}

void clean_quad_buf(QuadBufGLIDs &glIDs)
{
    glDeleteBuffers(1, &glIDs.vertBufGLID);
    glDeleteBuffers(1, &glIDs.elemBufGLID);
    glDeleteVertexArrays(1, &glIDs.vertArrayGLID);

    glIDs = {};
}

void init_renderer(Renderer &renderer, const int layerCnt, const int camLayerCnt, const RenderLayerFactory layerFactory)
{
    assert(layerCnt > 0 && layerCnt <= gk_renderLayerLimit);
    assert(camLayerCnt >= 0 && camLayerCnt <= layerCnt);
    assert(layerFactory);

    renderer.layerCnt = layerCnt;
    renderer.camLayerCnt = camLayerCnt;

    for (int i = 0; i < layerCnt; ++i)
    {
        renderer.layers[i] = make_render_layer(layerFactory(i));
    }
}

void clean_renderer(Renderer &renderer)
{
    for (int i = 0; i < renderer.layerCnt; ++i)
    {
        clean_render_layer(renderer.layers[i]);
    }

    renderer = {};
}

void draw_render_layers(const Renderer &renderer, const Color &bgColor, const AssetGroupManager &assetGroupManager, const ShaderProgGLIDs &shaderProgGLIDs, const Camera *const cam, const cc::Vec2DInt windowSize)
{
    assert((renderer.camLayerCnt > 0) == (cam != nullptr));

    glClearColor(bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    glClear(GL_COLOR_BUFFER_BIT);

    const int texUnitLimit = get_tex_unit_limit();

    int texUnits[gk_texUnitLimitCap];
    std::iota(texUnits, texUnits + texUnitLimit, 0);

    // Create the projection and view matrices.
    const auto projMat = cc::make_ortho_matrix(0.0f, windowSize.x, windowSize.y, 0.0f, -1.0f, 1.0f);

    // Define function for rendering a layer.
    auto renderLayer = [&assetGroupManager, &shaderProgGLIDs, texUnitLimit, texUnits, &projMat](const RenderLayer &layer, const cc::Matrix4x4 &viewMat)
    {
        // Render sprite batches.
        glUseProgram(shaderProgGLIDs.spriteQuadGLID);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgGLIDs.spriteQuadGLID, "u_proj"), 1, false, reinterpret_cast<const float *>(projMat.elems));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgGLIDs.spriteQuadGLID, "u_view"), 1, false, reinterpret_cast<const float *>(viewMat.elems));

        glUniform1iv(glGetUniformLocation(shaderProgGLIDs.spriteQuadGLID, "u_textures"), texUnitLimit, texUnits);

        for (int i = 0; i < layer.spriteBatchCnt; ++i)
        {
            if (!is_bit_active(layer.spriteBatchActivity, i))
            {
                continue;
            }

            // Bind texture GLIDs to units.
            for (int j = 0; j < texUnitLimit; ++j)
            {
                const SpriteBatchTexUnitInfo &texUnit = layer.spriteBatches.texUnitInfos[(i * texUnitLimit) + j];
                glActiveTexture(GL_TEXTURE0 + j);
                glBindTexture(GL_TEXTURE_2D, texUnit.refCnt > 0 ? assetGroupManager.get_tex_gl_id(texUnit.texID) : 0);
            }

            // Draw the batch.
            glBindVertexArray(layer.spriteBatches.quadBufGLIDs[i].vertArrayGLID);
            glDrawElements(GL_TRIANGLES, 6 * layer.spriteBatchSlotCnt, GL_UNSIGNED_SHORT, nullptr);
        }

        // Render character batches.
        glUseProgram(shaderProgGLIDs.charQuadGLID);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgGLIDs.charQuadGLID, "u_proj"), 1, false, reinterpret_cast<const float *>(projMat.elems));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgGLIDs.charQuadGLID, "u_view"), 1, false, reinterpret_cast<const float *>(viewMat.elems));

        for (int i = 0; i < layer.charBatchCnt; ++i)
        {
            if (!is_bit_active(layer.charBatchActivity, i))
            {
                continue;
            }

            const CharBatchDisplayProps &displayProps = layer.charBatches.displayProps[i];

            glUniform2fv(glGetUniformLocation(shaderProgGLIDs.charQuadGLID, "u_pos"), 1, reinterpret_cast<const float *>(&displayProps.pos));
            glUniform1f(glGetUniformLocation(shaderProgGLIDs.charQuadGLID, "u_rot"), displayProps.rot);
            glUniform4fv(glGetUniformLocation(shaderProgGLIDs.charQuadGLID, "u_blend"), 1, reinterpret_cast<const float *>(&displayProps.blend));

            // Bind font texture GLID.
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, assetGroupManager.get_font_tex_gl_id(layer.charBatches.fontIDs[i]));

            // Draw the batch.
            glBindVertexArray(layer.charBatches.quadBufGLIDs[i].vertArrayGLID);
            glDrawElements(GL_TRIANGLES, 6 * layer.charBatches.slotCnts[i], GL_UNSIGNED_SHORT, nullptr);
        }
    };

    // Render camera layers then non-camera ones.
    if (renderer.camLayerCnt > 0)
    {
        const cc::Matrix4x4 camViewMat = make_camera_view_matrix(*cam, windowSize);

        int i = 0;

        do {
            renderLayer(renderer.layers[i], camViewMat);
            ++i;
        }
        while (i < renderer.camLayerCnt);
    }

    const cc::Matrix4x4 defaultViewMat = cc::make_identity_matrix();

    for (int i = renderer.camLayerCnt; i < renderer.layerCnt; ++i)
    {
        renderLayer(renderer.layers[i], defaultViewMat);
    }
}

RenderLayer make_render_layer(const RenderLayerCreateInfo &createInfo)
{
    assert(createInfo.spriteBatchCnt > 0 && createInfo.spriteBatchCnt <= gk_renderLayerSpriteBatchLimit);
    assert(createInfo.spriteBatchSlotCnt > 0 && createInfo.spriteBatchSlotCnt <= gk_spriteBatchSlotLimit);
    assert(createInfo.charBatchCnt > 0 && createInfo.charBatchCnt <= gk_renderLayerCharBatchLimit);

    RenderLayer layer = {
        .spriteBatchCnt = createInfo.spriteBatchCnt,
        .spriteBatchSlotCnt = createInfo.spriteBatchSlotCnt,
        .charBatchCnt = createInfo.charBatchCnt
    };

    layer.spriteBatches.quadBufGLIDs = cc::push_to_mem_arena<QuadBufGLIDs>(g_permMemArena, createInfo.spriteBatchCnt);
    layer.spriteBatches.slotTexUnits = cc::push_to_mem_arena<int>(g_permMemArena, createInfo.spriteBatchSlotCnt * createInfo.spriteBatchCnt);
    layer.spriteBatches.texUnitInfos = cc::push_to_mem_arena<SpriteBatchTexUnitInfo>(g_permMemArena, get_tex_unit_limit() * createInfo.spriteBatchCnt);

    for (int i = 0; i < createInfo.spriteBatchCnt; ++i)
    {
        init_heap_bitset(layer.spriteBatches.slotActivity[i], g_permMemArena, createInfo.spriteBatchSlotCnt);
    }

    layer.charBatches.slotCnts = cc::push_to_mem_arena<int>(g_permMemArena, createInfo.charBatchCnt);
    layer.charBatches.quadBufGLIDs = cc::push_to_mem_arena<QuadBufGLIDs>(g_permMemArena, createInfo.charBatchCnt);
    layer.charBatches.fontIDs = cc::push_to_mem_arena<AssetID>(g_permMemArena, createInfo.charBatchCnt);
    layer.charBatches.displayProps = cc::push_to_mem_arena<CharBatchDisplayProps>(g_permMemArena, createInfo.charBatchCnt);

    init_heap_bitset(layer.spriteBatchActivity, g_permMemArena, createInfo.spriteBatchCnt);
    init_heap_bitset(layer.charBatchActivity, g_permMemArena, createInfo.charBatchCnt);

    return layer;
}

void clean_render_layer(RenderLayer &layer)
{
    for (int i = 0; i < layer.spriteBatchCnt; ++i)
    {
        if (!is_bit_active(layer.spriteBatchActivity, i))
        {
            continue;
        }

        clean_quad_buf(layer.spriteBatches.quadBufGLIDs[i]);
    }

    for (int i = 0; i < layer.charBatchCnt; ++i)
    {
        if (!is_bit_active(layer.charBatchActivity, i))
        {
            continue;
        }

        clean_quad_buf(layer.charBatches.quadBufGLIDs[i]);
    }

    layer = {};
}

SpriteBatchSlotKey take_any_sprite_batch_slot(Renderer &renderer, const int layerIndex, const AssetID texID)
{
    assert(layerIndex >= 0 && layerIndex < renderer.layerCnt);

    RenderLayer &layer = renderer.layers[layerIndex];

    for (int i = 0; i < layer.spriteBatchCnt; ++i)
    {
        if (!is_bit_active(layer.spriteBatchActivity, i))
        {
            continue;
        }

        // Continue if no slot is available.
        if (are_all_bits_active(layer.spriteBatches.slotActivity[i]))
        {
            continue;
        }

        // Find a texture unit to use, continue if none are suitable.
        const int texUnit = find_sprite_batch_tex_unit_to_use(layer, i, texID);

        if (texUnit == -1)
        {
            continue;
        }

        // Use the first inactive slot.
        const int slotIndex = first_inactive_bit_index(layer.spriteBatches.slotActivity[i]);
        activate_bit(layer.spriteBatches.slotActivity[i], slotIndex);

        // Update texture unit information.
        SpriteBatchTexUnitInfo &texUnitInfo = layer.spriteBatches.texUnitInfos[(i * get_tex_unit_limit()) + texUnit];
        texUnitInfo.texID = texID;
        ++texUnitInfo.refCnt;

        layer.spriteBatches.slotTexUnits[(i * layer.spriteBatchSlotCnt) + slotIndex] = texUnit;

        return {
            .layerIndex = layerIndex,
            .batchIndex = i,
            .slotIndex = slotIndex
        };
    }

    // Failed to find a batch to use in the layer, so activate a new one and try this all again.
    activate_any_sprite_batch(renderer, layerIndex);
    return take_any_sprite_batch_slot(renderer, layerIndex, texID); // TEMP: Way more work is done here than necessary.
}

void release_sprite_batch_slot(Renderer &renderer, const SpriteBatchSlotKey &key)
{
    RenderLayer &layer = renderer.layers[key.layerIndex];

    // Mark the slot as inactive.
    deactivate_bit(layer.spriteBatches.slotActivity[key.batchIndex], key.slotIndex);

    // Update texture unit information.
    const int texUnit = layer.spriteBatches.slotTexUnits[(key.batchIndex * layer.spriteBatchSlotCnt) + key.slotIndex];
    SpriteBatchTexUnitInfo &texUnitInfo = layer.spriteBatches.texUnitInfos[(key.batchIndex * get_tex_unit_limit()) + texUnit];
    --texUnitInfo.refCnt;

    // Clear the slot render data.
    clear_sprite_batch_slot(renderer, key);
}

void write_to_sprite_batch_slot(const Renderer &renderer, const SpriteBatchSlotKey &key, const SpriteBatchSlotWriteData &writeData, const AssetGroupManager &assetGroupManager)
{
    const RenderLayer &layer = renderer.layers[key.layerIndex];

    const int texUnit = layer.get_sprite_batch_slot_tex_unit(key.batchIndex, key.slotIndex);
    const SpriteBatchTexUnitInfo &texUnitInfo = layer.get_sprite_batch_tex_unit_info(key.batchIndex, texUnit);
    const cc::Vec2DInt texSize = assetGroupManager.get_tex_size(texUnitInfo.texID);

    const float verts[gk_spriteQuadShaderProgVertCnt * 4] = {
        (0.0f - writeData.origin.x) * writeData.scale.x,
        (0.0f - writeData.origin.y) * writeData.scale.y,
        writeData.pos.x,
        writeData.pos.y,
        static_cast<float>(writeData.srcRect.width),
        static_cast<float>(writeData.srcRect.height),
        writeData.rot,
        static_cast<float>(texUnit),
        static_cast<float>(writeData.srcRect.x) / texSize.x,
        static_cast<float>(writeData.srcRect.y) / texSize.y,
        writeData.alpha,

        (1.0f - writeData.origin.x) * writeData.scale.x,
        (0.0f - writeData.origin.y) * writeData.scale.y,
        writeData.pos.x,
        writeData.pos.y,
        static_cast<float>(writeData.srcRect.width),
        static_cast<float>(writeData.srcRect.height),
        writeData.rot,
        static_cast<float>(texUnit),
        static_cast<float>(writeData.srcRect.x + writeData.srcRect.width) / texSize.x,
        static_cast<float>(writeData.srcRect.y) / texSize.y,
        writeData.alpha,

        (1.0f - writeData.origin.x) * writeData.scale.x,
        (1.0f - writeData.origin.y) * writeData.scale.y,
        writeData.pos.x,
        writeData.pos.y,
        static_cast<float>(writeData.srcRect.width),
        static_cast<float>(writeData.srcRect.height),
        writeData.rot,
        static_cast<float>(texUnit),
        static_cast<float>(writeData.srcRect.x + writeData.srcRect.width) / texSize.x,
        static_cast<float>(writeData.srcRect.y + writeData.srcRect.height) / texSize.y,
        writeData.alpha,

        (0.0f - writeData.origin.x) * writeData.scale.x,
        (1.0f - writeData.origin.y) * writeData.scale.y,
        writeData.pos.x,
        writeData.pos.y,
        static_cast<float>(writeData.srcRect.width),
        static_cast<float>(writeData.srcRect.height),
        writeData.rot,
        static_cast<float>(texUnit),
        static_cast<float>(writeData.srcRect.x) / texSize.x,
        static_cast<float>(writeData.srcRect.y + writeData.srcRect.height) / texSize.y,
        writeData.alpha
    };

    QuadBufGLIDs &quadBufGLIDs = renderer.layers[key.layerIndex].spriteBatches.quadBufGLIDs[key.batchIndex];

    glBindVertexArray(quadBufGLIDs.vertArrayGLID);
    glBindBuffer(GL_ARRAY_BUFFER, quadBufGLIDs.vertBufGLID);
    glBufferSubData(GL_ARRAY_BUFFER, key.slotIndex * sizeof(verts), sizeof(verts), verts);
}

void clear_sprite_batch_slot(const Renderer &renderer, const SpriteBatchSlotKey &key)
{
    QuadBufGLIDs &quadBufGLIDs = renderer.layers[key.layerIndex].spriteBatches.quadBufGLIDs[key.batchIndex];

    glBindVertexArray(quadBufGLIDs.vertArrayGLID);
    glBindBuffer(GL_ARRAY_BUFFER, quadBufGLIDs.vertBufGLID);

    const float verts[gk_spriteQuadShaderProgVertCnt * 4] = {};
    glBufferSubData(GL_ARRAY_BUFFER, key.slotIndex * sizeof(verts), sizeof(verts), verts);
}

CharBatchKey activate_any_char_batch(Renderer &renderer, const int layerIndex, const int slotCnt, const AssetID fontID, const AssetGroupManager &assetGroupManager)
{
    assert(layerIndex >= 0 && layerIndex < renderer.layerCnt);
    assert(slotCnt > 0 && slotCnt <= gk_charBatchSlotLimit);

    RenderLayer &layer = renderer.layers[layerIndex];

    const int batchIndex = first_inactive_bit_index(layer.charBatchActivity);
    assert(batchIndex != -1);

    activate_bit(layer.charBatchActivity, batchIndex);

    layer.charBatches.slotCnts[batchIndex] = slotCnt;
    layer.charBatches.quadBufGLIDs[batchIndex] = make_quad_buf(slotCnt, false);
    layer.charBatches.fontIDs[batchIndex] = fontID;
    layer.charBatches.displayProps[batchIndex] = {
        .pos = {},
        .rot = 0.0f,
        .blend = Color::make_white()
    };

    return {
        .layerIndex = layerIndex,
        .batchIndex = batchIndex
    };
}

void deactivate_char_batch(Renderer &renderer, const CharBatchKey &key)
{
    RenderLayer &layer = renderer.layers[key.layerIndex];
    deactivate_bit(layer.charBatchActivity, key.batchIndex);
    clear_char_batch(renderer, key);
}

void write_to_char_batch(Renderer &renderer, const CharBatchKey &key, const char *const text, const FontHorAlign horAlign, const FontVerAlign verAlign, const AssetGroupManager &assetGroupManager)
{
    RenderLayer &layer = renderer.layers[key.layerIndex];
    const int slotCnt = layer.charBatches.slotCnts[key.batchIndex];

    const int textLen = strlen(text);
    assert(textLen > 0 && textLen <= slotCnt);

    const cc::FontDisplayInfo &fontDisplayInfo = assetGroupManager.get_font_display_info(layer.charBatches.fontIDs[key.batchIndex]);

    // Determine the positions of text characters based on font information, alongside the overall dimensions of the text to be used when applying alignment.
    const auto charDrawPositions = cc::push_to_mem_arena<cc::Vec2D>(g_tempMemArena, slotCnt);
    cc::Vec2D charDrawPosPen = {};

    const auto textLineWidths = cc::push_to_mem_arena<int>(g_tempMemArena, slotCnt + 1);
    int textFirstLineMinOffs = 0;
    bool textFirstLineMinOffsUpdated = false;
    int textLastLineMaxHeight = 0;
    bool textLastLineMaxHeightUpdated = false;
    int textLineCnter = 0;

    for (int i = 0; i < textLen; i++)
    {
        if (text[i] == '\n')
        {
            textLineWidths[textLineCnter] = charDrawPosPen.x;

            if (!textFirstLineMinOffsUpdated)
            {
                // Set the first line minimum offset to the vertical offset of the space character.
                textFirstLineMinOffs = fontDisplayInfo.chars.verOffsets[0];
                textFirstLineMinOffsUpdated = true;
            }

            // Set the last line maximum height to the height of a space.
            textLastLineMaxHeight = fontDisplayInfo.chars.verOffsets[0] + fontDisplayInfo.chars.srcRects[0].height;

            textLastLineMaxHeightUpdated = false;

            textLineCnter++;

            // Move the pen to a new line.
            charDrawPosPen.x = 0.0f;
            charDrawPosPen.y += fontDisplayInfo.lineHeight;

            continue;
        }

        const int textCharIndex = text[i] - cc::gk_fontCharRangeBegin;

        // If we are on the first line, update the first line minimum offset.
        if (textLineCnter == 0)
        {
            if (!textFirstLineMinOffsUpdated)
            {
                textFirstLineMinOffs = fontDisplayInfo.chars.verOffsets[textCharIndex];
                textFirstLineMinOffsUpdated = true;
            }
            else
            {
                textFirstLineMinOffs = std::min(fontDisplayInfo.chars.verOffsets[textCharIndex], textFirstLineMinOffs);
            }
        }

        if (!textLastLineMaxHeightUpdated)
        {
            textLastLineMaxHeight = fontDisplayInfo.chars.verOffsets[textCharIndex] + fontDisplayInfo.chars.srcRects[textCharIndex].height;
            textLastLineMaxHeightUpdated = true;
        }
        else
        {
            textLastLineMaxHeight = std::max(fontDisplayInfo.chars.verOffsets[textCharIndex] + fontDisplayInfo.chars.srcRects[textCharIndex].height, textLastLineMaxHeight);
        }

        if (i > 0)
        {
            // Apply kerning based on the previous character.
            const int textCharIndexLast = text[i - 1] - cc::gk_fontCharRangeBegin;
            charDrawPosPen.x += fontDisplayInfo.chars.kernings[(textCharIndex * cc::gk_fontCharRangeSize) + textCharIndexLast];
        }

        charDrawPositions[i].x = charDrawPosPen.x + fontDisplayInfo.chars.horOffsets[textCharIndex];
        charDrawPositions[i].y = charDrawPosPen.y + fontDisplayInfo.chars.verOffsets[textCharIndex];

        charDrawPosPen.x += fontDisplayInfo.chars.horAdvances[textCharIndex];
    }

    textLineWidths[textLineCnter] = charDrawPosPen.x;
    textLineCnter = 0;

    const int textHeight = textFirstLineMinOffs + charDrawPosPen.y + textLastLineMaxHeight;

    // Clear the batch so it can have only the new characters.
    clear_char_batch(renderer, key);

    // Bind the vertex array and buffer.
    const QuadBufGLIDs &quadBufGLIDs = layer.charBatches.quadBufGLIDs[key.batchIndex];
    glBindVertexArray(quadBufGLIDs.vertArrayGLID);
    glBindBuffer(GL_ARRAY_BUFFER, quadBufGLIDs.vertBufGLID);

    // Write character render data.
    for (int i = 0; i < textLen; i++)
    {
        if (text[i] == '\n')
        {
            textLineCnter++;
            continue;
        }

        if (text[i] == ' ')
        {
            continue;
        }

        const int charIndex = text[i] - cc::gk_fontCharRangeBegin;

        const cc::Vec2D charDrawPos = {
            charDrawPositions[i].x - (textLineWidths[textLineCnter] * horAlign * 0.5f),
            charDrawPositions[i].y - (textHeight * verAlign * 0.5f)
        };

        const cc::RectFloat charTexCoordsRect = {
            static_cast<float>(fontDisplayInfo.chars.srcRects[charIndex].x) / fontDisplayInfo.texSize.x,
            static_cast<float>(fontDisplayInfo.chars.srcRects[charIndex].y) / fontDisplayInfo.texSize.y,
            static_cast<float>(fontDisplayInfo.chars.srcRects[charIndex].width) / fontDisplayInfo.texSize.x,
            static_cast<float>(fontDisplayInfo.chars.srcRects[charIndex].height) / fontDisplayInfo.texSize.y
        };

        const float verts[gk_charQuadShaderProgVertCnt * 4] = {
            charDrawPos.x,
            charDrawPos.y,
            charTexCoordsRect.x,
            charTexCoordsRect.y,

            charDrawPos.x + fontDisplayInfo.chars.srcRects[charIndex].width,
            charDrawPos.y,
            charTexCoordsRect.x + charTexCoordsRect.width,
            charTexCoordsRect.y,

            charDrawPos.x + fontDisplayInfo.chars.srcRects[charIndex].width,
            charDrawPos.y + fontDisplayInfo.chars.srcRects[charIndex].height,
            charTexCoordsRect.x + charTexCoordsRect.width,
            charTexCoordsRect.y + charTexCoordsRect.height,

            charDrawPos.x,
            charDrawPos.y + fontDisplayInfo.chars.srcRects[charIndex].height,
            charTexCoordsRect.x,
            charTexCoordsRect.y + charTexCoordsRect.height
        };

        glBufferSubData(GL_ARRAY_BUFFER, i * sizeof(verts), sizeof(verts), verts);
    }
}

void clear_char_batch(const Renderer &renderer, const CharBatchKey &key)
{
    const RenderLayer &layer = renderer.layers[key.layerIndex];
    const QuadBufGLIDs &quadBufGLIDs = layer.charBatches.quadBufGLIDs[key.batchIndex];

    glBindVertexArray(quadBufGLIDs.vertArrayGLID);
    glBindBuffer(GL_ARRAY_BUFFER, quadBufGLIDs.vertBufGLID);

    const float verts[(gk_charQuadShaderProgVertCnt * 4) * gk_renderLayerCharBatchLimit] = {};
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
}
