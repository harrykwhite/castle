#include "c_rendering.h"

#include <numeric>
#include "c_game.h"

TexUnit g_texUnitLimit;

static void init_render_layer(RenderLayer &layer, const RenderLayerInitInfo &initInfo)
{
    assert(initInfo.spriteBatchCnt >= 0);
    assert(initInfo.spriteBatchSlotCnt >= 0);
    assert(initInfo.charBatchCnt >= 0);

    // Initialise sprite batches.
    layer.spriteBatches = cc::push_to_mem_arena<SpriteBatch>(g_permMemArena, initInfo.spriteBatchCnt);
    layer.spriteBatchCnt = initInfo.spriteBatchCnt;
    layer.spriteBatchSlotCnt = initInfo.spriteBatchSlotCnt;
    layer.spriteBatchActivity = cc::push_to_mem_arena<cc::Byte>(g_permMemArena, bits_to_bytes(initInfo.spriteBatchCnt));

    for (int i = 0; i < initInfo.spriteBatchCnt; ++i)
    {
        SpriteBatch &sb = layer.spriteBatches[i];
        sb.quadBufVerts = cc::push_to_mem_arena<float>(g_permMemArena, gk_spriteBatchSlotVertsSize * initInfo.spriteBatchSlotCnt);
        sb.slotActivity = cc::push_to_mem_arena<cc::Byte>(g_permMemArena, bits_to_bytes(initInfo.spriteBatchSlotCnt));
        sb.slotTexUnits = cc::push_to_mem_arena<TexUnit>(g_permMemArena, initInfo.spriteBatchSlotCnt);
        sb.texUnitInfos = cc::push_to_mem_arena<SpriteBatchTexUnitInfo>(g_permMemArena, g_texUnitLimit);
    }

    // Initialise character batches.
    layer.charBatches = cc::push_to_mem_arena<CharBatch>(g_permMemArena, initInfo.charBatchCnt);
    layer.charBatchCnt = initInfo.charBatchCnt;
    layer.charBatchActivity = cc::push_to_mem_arena<cc::Byte>(g_permMemArena, bits_to_bytes(initInfo.charBatchCnt));
}

static void clean_render_layer(RenderLayer &layer)
{
    for (int i = 0; i < layer.spriteBatchCnt; ++i)
    {
        if (!is_bit_active(layer.spriteBatchActivity, i))
        {
            continue;
        }

        clean_quad_buf(layer.spriteBatches[i].quadBufGLIDs);
    }

    for (int i = 0; i < layer.charBatchCnt; ++i)
    {
        if (!is_bit_active(layer.charBatchActivity, i))
        {
            continue;
        }

        clean_quad_buf(layer.charBatches[i].quadBufGLIDs);
    }

    layer = {};
}

static TexUnit find_sprite_batch_tex_unit_to_use(const RenderLayer &renderLayer, const int batchIndex, const AssetID texID)
{
    TexUnit freeTexUnit = -1;

    for (int i = 0; i < g_texUnitLimit; ++i)
    {
        const SpriteBatchTexUnitInfo &texUnitInfo = renderLayer.spriteBatches[batchIndex].texUnitInfos[i];

        if (!texUnitInfo.refCnt)
        {
            if (freeTexUnit == -1)
            {
                freeTexUnit = i;
            }

            continue;
        }

        if (texUnitInfo.texID == texID)
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

    const int batchIndex = first_inactive_bit_index(layer.spriteBatchActivity, layer.spriteBatchCnt);

    if (batchIndex == -1)
    {
        assert(false);
        return;
    }

    activate_bit(layer.spriteBatchActivity, batchIndex);

    SpriteBatch &batch = layer.spriteBatches[batchIndex];

    batch.quadBufGLIDs = make_quad_buf(layer.spriteBatchSlotCnt, true);
    memset(batch.quadBufVerts, 0, gk_spriteBatchSlotVertsSize * layer.spriteBatchSlotCnt * sizeof(float));
    clear_bits(batch.slotActivity, layer.spriteBatchSlotCnt);
    memset(batch.slotTexUnits, 0, layer.spriteBatchSlotCnt * sizeof(TexUnit));
    batch.modifiedSlotRange = {};
    memset(batch.texUnitInfos, 0, g_texUnitLimit * sizeof(SpriteBatchTexUnitInfo));
}

void init_tex_unit_limit()
{
    int limit;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &limit);

    g_texUnitLimit = std::min(limit, gk_texUnitLimitCap);
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
        memset(verts, 0, sizeof(verts[0]) * vertsLen);
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

void init_renderer(Renderer &renderer, const int layerCnt, const int camLayerCnt, const RenderLayerInitInfoFactory layerInitInfoFactory)
{
    assert(layerCnt > 0);
    assert(camLayerCnt >= 0 && camLayerCnt <= layerCnt);
    assert(layerInitInfoFactory);

    renderer.layerCnt = layerCnt;
    renderer.camLayerCnt = camLayerCnt;

    renderer.layers = cc::push_to_mem_arena<RenderLayer>(g_permMemArena, layerCnt);

    for (int i = 0; i < layerCnt; ++i)
    {
        init_render_layer(renderer.layers[i], layerInitInfoFactory(i));
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

void render(const Renderer &renderer, const Color &bgColor, const AssetGroupManager &assetGroupManager, const ShaderProgs &shaderProgs, const Camera *const cam)
{
    assert((renderer.camLayerCnt > 0) == (cam != nullptr));

    // Clear the screen with the background colour.
    glClearColor(bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set up texture units.
    LOCAL_PERSIST int texUnits[gk_texUnitLimitCap];
    LOCAL_PERSIST bool texUnitsInitialized;

    if (!texUnitsInitialized)
    {
        std::iota(texUnits, texUnits + g_texUnitLimit, 0);
        texUnitsInitialized = true;
    }

    // Create the projection matrices.
    const auto projMat = cc::make_ortho_matrix_4x4(0.0f, get_window_size().x, get_window_size().y, 0.0f, -1.0f, 1.0f);

    // Define function for rendering a layer.
    auto renderLayer = [&assetGroupManager, &shaderProgs, &projMat](const RenderLayer &layer, const cc::Matrix4x4 &viewMat)
    {
        // Render sprite batches.
        glUseProgram(shaderProgs.spriteQuadGLID);

        glUniformMatrix4fv(shaderProgs.spriteQuadProjUniLoc, 1, false, reinterpret_cast<const float *>(projMat.elems));
        glUniformMatrix4fv(shaderProgs.spriteQuadViewUniLoc, 1, false, reinterpret_cast<const float *>(viewMat.elems));

        glUniform1iv(shaderProgs.spriteQuadTexturesUniLoc, g_texUnitLimit, texUnits);

        for (int i = 0; i < layer.spriteBatchCnt; ++i)
        {
            if (!is_bit_active(layer.spriteBatchActivity, i))
            {
                continue;
            }

            const SpriteBatch &sb = layer.spriteBatches[i];

            // Bind texture GLIDs to units.
            for (int j = 0; j < g_texUnitLimit; ++j)
            {
                if (!sb.texUnitInfos[j].refCnt)
                {
                    continue;
                }

                glActiveTexture(GL_TEXTURE0 + j);
                glBindTexture(GL_TEXTURE_2D, assetGroupManager.get_tex_gl_id(sb.texUnitInfos[j].texID));
            }

            // Draw the batch.
            glBindVertexArray(sb.quadBufGLIDs.vertArrayGLID);
            glDrawElements(GL_TRIANGLES, 6 * layer.spriteBatchSlotCnt, GL_UNSIGNED_SHORT, nullptr);
        }

        // Render character batches.
        glUseProgram(shaderProgs.charQuadGLID);

        glUniformMatrix4fv(shaderProgs.charQuadProjUniLoc, 1, false, reinterpret_cast<const float *>(projMat.elems));
        glUniformMatrix4fv(shaderProgs.charQuadViewUniLoc, 1, false, reinterpret_cast<const float *>(viewMat.elems));

        for (int i = 0; i < layer.charBatchCnt; ++i)
        {
            if (!is_bit_active(layer.charBatchActivity, i))
            {
                continue;
            }

            CharBatch &cb = layer.charBatches[i];

            glUniform2fv(shaderProgs.charQuadPosUniLoc, 1, reinterpret_cast<const float *>(&cb.pos));
            glUniform1f(shaderProgs.charQuadRotUniLoc, cb.rot);
            glUniform4fv(shaderProgs.charQuadBlendUniLoc, 1, reinterpret_cast<const float *>(&cb.blend));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, assetGroupManager.get_font_tex_gl_id(cb.fontID));

            // Draw the batch.
            glBindVertexArray(cb.quadBufGLIDs.vertArrayGLID);
            glDrawElements(GL_TRIANGLES, 6 * cb.slotCnt, GL_UNSIGNED_SHORT, nullptr);
        }
    };

    // Render camera layers then non-camera ones.
    if (renderer.camLayerCnt > 0)
    {
        const cc::Matrix4x4 camViewMat = make_camera_view_matrix(*cam);

        int i = 0;

        do
        {
            renderLayer(renderer.layers[i], camViewMat);
            ++i;
        }
        while (i < renderer.camLayerCnt);
    }

    const cc::Matrix4x4 defaultViewMat = cc::make_identity_matrix_4x4();

    for (int i = renderer.camLayerCnt; i < renderer.layerCnt; ++i)
    {
        renderLayer(renderer.layers[i], defaultViewMat);
    }
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

        SpriteBatch &sb = layer.spriteBatches[i];

        // Continue if no slot is available.
        if (are_all_bits_active(sb.slotActivity, layer.spriteBatchSlotCnt))
        {
            continue;
        }

        // Find a texture unit to use, continue if none are suitable.
        const TexUnit texUnit = find_sprite_batch_tex_unit_to_use(layer, i, texID);

        if (texUnit == -1)
        {
            continue;
        }

        // Use the first inactive slot.
        const int slotIndex = first_inactive_bit_index(sb.slotActivity, layer.spriteBatchSlotCnt);
        activate_bit(sb.slotActivity, slotIndex);

        // Update texture unit information.
        SpriteBatchTexUnitInfo &texUnitInfo = sb.texUnitInfos[texUnit];
        texUnitInfo.texID = texID;
        ++texUnitInfo.refCnt;

        sb.slotTexUnits[slotIndex] = texUnit;

        return {
            .layerIndex = layerIndex,
            .batchIndex = i,
            .slotIndex = slotIndex
        };
    }

    // Failed to find a batch to use in the layer, so activate a new one and try this all again.
    activate_any_sprite_batch(renderer, layerIndex);
    return take_any_sprite_batch_slot(renderer, layerIndex, texID); // FIXME: Way more work is done here than necessary.
}

void release_sprite_batch_slot(Renderer &renderer, const SpriteBatchSlotKey &key)
{
    RenderLayer &layer = renderer.layers[key.layerIndex];
    SpriteBatch &batch = layer.spriteBatches[key.batchIndex];

    // Mark the slot as inactive.
    deactivate_bit(batch.slotActivity, layer.spriteBatchSlotCnt);

    // Update texture unit information.
    const TexUnit texUnit = batch.slotTexUnits[key.slotIndex];
    --batch.texUnitInfos[texUnit].refCnt;

    // Clear the slot render data.
    clear_sprite_batch_slot(renderer, key);
}

void write_to_sprite_batch_slot(Renderer &renderer, const SpriteBatchSlotKey &key, const SpriteBatchSlotWriteData &writeData, const AssetGroupManager &assetGroupManager)
{
    RenderLayer &layer = renderer.layers[key.layerIndex];
    SpriteBatch &batch = layer.spriteBatches[key.batchIndex];
    const int texUnit = batch.slotTexUnits[key.slotIndex];
    const cc::Vec2DInt texSize = assetGroupManager.get_tex_size(batch.texUnitInfos[texUnit].texID);

    float *const verts = batch.quadBufVerts + (key.slotIndex * gk_spriteBatchSlotVertsSize);

    verts[0] = (0.0f - writeData.origin.x) * writeData.scale.x;
    verts[1] = (0.0f - writeData.origin.y) * writeData.scale.y;
    verts[2] = writeData.pos.x;
    verts[3] = writeData.pos.y;
    verts[4] = static_cast<float>(writeData.srcRect.width);
    verts[5] = static_cast<float>(writeData.srcRect.height);
    verts[6] = writeData.rot;
    verts[7] = static_cast<float>(texUnit);
    verts[8] = static_cast<float>(writeData.srcRect.x) / texSize.x;
    verts[9] = static_cast<float>(writeData.srcRect.y) / texSize.y;
    verts[10] = writeData.alpha;

    verts[11] = (1.0f - writeData.origin.x) * writeData.scale.x;
    verts[12] = (0.0f - writeData.origin.y) * writeData.scale.y;
    verts[13] = writeData.pos.x;
    verts[14] = writeData.pos.y;
    verts[15] = static_cast<float>(writeData.srcRect.width);
    verts[16] = static_cast<float>(writeData.srcRect.height);
    verts[17] = writeData.rot;
    verts[18] = static_cast<float>(texUnit);
    verts[19] = static_cast<float>(writeData.srcRect.x + writeData.srcRect.width) / texSize.x;
    verts[20] = static_cast<float>(writeData.srcRect.y) / texSize.y;
    verts[21] = writeData.alpha;

    verts[22] = (1.0f - writeData.origin.x) * writeData.scale.x;
    verts[23] = (1.0f - writeData.origin.y) * writeData.scale.y;
    verts[24] = writeData.pos.x;
    verts[25] = writeData.pos.y;
    verts[26] = static_cast<float>(writeData.srcRect.width);
    verts[27] = static_cast<float>(writeData.srcRect.height);
    verts[28] = writeData.rot;
    verts[29] = static_cast<float>(texUnit);
    verts[30] = static_cast<float>(writeData.srcRect.x + writeData.srcRect.width) / texSize.x;
    verts[31] = static_cast<float>(writeData.srcRect.y + writeData.srcRect.height) / texSize.y;
    verts[32] = writeData.alpha;

    verts[33] = (0.0f - writeData.origin.x) * writeData.scale.x;
    verts[34] = (1.0f - writeData.origin.y) * writeData.scale.y;
    verts[35] = writeData.pos.x;
    verts[36] = writeData.pos.y;
    verts[37] = static_cast<float>(writeData.srcRect.width);
    verts[38] = static_cast<float>(writeData.srcRect.height);
    verts[39] = writeData.rot;
    verts[40] = static_cast<float>(texUnit);
    verts[41] = static_cast<float>(writeData.srcRect.x) / texSize.x;
    verts[42] = static_cast<float>(writeData.srcRect.y + writeData.srcRect.height) / texSize.y;
    verts[43] = writeData.alpha;

    batch.modifiedSlotRange.begin = std::min(batch.modifiedSlotRange.begin, key.slotIndex);
    batch.modifiedSlotRange.end = std::max(batch.modifiedSlotRange.end, key.slotIndex + 1);
}

void clear_sprite_batch_slot(const Renderer &renderer, const SpriteBatchSlotKey &key)
{
    const RenderLayer &layer = renderer.layers[key.layerIndex];
    const SpriteBatch &batch = layer.spriteBatches[key.batchIndex];

    glBindVertexArray(batch.quadBufGLIDs.vertArrayGLID);
    glBindBuffer(GL_ARRAY_BUFFER, batch.quadBufGLIDs.vertBufGLID);

    const float verts[gk_spriteBatchSlotVertsSize] = {};
    glBufferSubData(GL_ARRAY_BUFFER, key.slotIndex * sizeof(verts), sizeof(verts), verts);
}

void submit_sprite_batch_slots(Renderer &renderer)
{
    for (int i = 0; i < renderer.layerCnt; ++i)
    {
        const RenderLayer &layer = renderer.layers[i];

        for (int j = 0; j < layer.spriteBatchCnt; ++j)
        {
            SpriteBatch &batch = layer.spriteBatches[j];

            if (!is_bit_active(layer.spriteBatchActivity, j))
            {
                continue;
            }

            if (batch.modifiedSlotRange.end - batch.modifiedSlotRange.begin == 0)
            {
                // No slots have been modified, so no need to write.
                continue;
            }

            // Submit the modified range of vertex data.
            glBindVertexArray(batch.quadBufGLIDs.vertArrayGLID);
            glBindBuffer(GL_ARRAY_BUFFER, batch.quadBufGLIDs.vertBufGLID);

            const int offs = (gk_spriteBatchSlotVertsSize)*batch.modifiedSlotRange.begin * sizeof(float);
            const int size = (gk_spriteBatchSlotVertsSize) * (batch.modifiedSlotRange.end - batch.modifiedSlotRange.begin) * sizeof(float);
            glBufferSubData(GL_ARRAY_BUFFER, offs, size, batch.quadBufVerts);

            // Reset the modified slot range for next time.
            batch.modifiedSlotRange = {};
        }
    }
}

CharBatchKey activate_any_char_batch(Renderer &renderer, const int layerIndex, const int slotCnt, const AssetID fontID, const cc::Vec2D pos, const AssetGroupManager &assetGroupManager)
{
    assert(layerIndex >= 0 && layerIndex < renderer.layerCnt);
    assert(slotCnt > 0);

    RenderLayer &layer = renderer.layers[layerIndex];

    const int batchIndex = first_inactive_bit_index(layer.charBatchActivity, layer.charBatchCnt);
    assert(batchIndex != -1);

    activate_bit(layer.charBatchActivity, batchIndex);

    CharBatch &batch = layer.charBatches[batchIndex];

    batch.quadBufGLIDs = make_quad_buf(slotCnt, false);
    batch.slotCnt = slotCnt;
    batch.fontID = fontID;
    batch.pos = pos;
    batch.rot = 0.0f;
    batch.blend = Color::make_white();

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
    CharBatch &batch = layer.charBatches[key.batchIndex];

    const int textLen = strlen(text);
    assert(textLen > 0 && textLen <= batch.slotCnt);

    const cc::FontDisplayInfo &fontDisplayInfo = assetGroupManager.get_font_display_info(batch.fontID);

    // Determine the positions of text characters based on font information, alongside the overall dimensions of the text to be used when applying alignment.
    const auto charDrawPositions = cc::push_to_mem_arena<cc::Vec2D>(g_tempMemArena, batch.slotCnt);
    cc::Vec2D charDrawPosPen = {};

    const auto textLineWidths = cc::push_to_mem_arena<int>(g_tempMemArena, batch.slotCnt + 1);
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

    // Reserve memory to hold the vertex data for the characters.
    const int vertsLen = gk_charBatchSlotVertsSize * textLen;
    const auto verts = cc::push_to_mem_arena<float>(g_tempMemArena, vertsLen);
    memset(verts, 0, vertsLen * sizeof(verts[0]));

    // Write the vertex data.
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

        const cc::Vec2D charTexCoordsTopLeft = {
            static_cast<float>(fontDisplayInfo.chars.srcRects[charIndex].x) / fontDisplayInfo.texSize.x,
            static_cast<float>(fontDisplayInfo.chars.srcRects[charIndex].y) / fontDisplayInfo.texSize.y
        };

        const cc::Vec2D charTexCoordsBottomRight = {
            static_cast<float>(fontDisplayInfo.chars.srcRects[charIndex].right() + 1.0f) / fontDisplayInfo.texSize.x, // FIXME: The +1.0f is a hack to fix the text being cut off. Still need to figure out why it's actually happening.
            static_cast<float>(fontDisplayInfo.chars.srcRects[charIndex].bottom()) / fontDisplayInfo.texSize.y
        };

        float *const slotVerts = verts + (i * gk_charBatchSlotVertsSize);

        slotVerts[0] = charDrawPos.x;
        slotVerts[1] = charDrawPos.y;
        slotVerts[2] = charTexCoordsTopLeft.x;
        slotVerts[3] = charTexCoordsTopLeft.y;

        slotVerts[4] = charDrawPos.x + fontDisplayInfo.chars.srcRects[charIndex].width + 1;
        slotVerts[5] = charDrawPos.y;
        slotVerts[6] = charTexCoordsBottomRight.x;
        slotVerts[7] = charTexCoordsTopLeft.y;

        slotVerts[8] = charDrawPos.x + fontDisplayInfo.chars.srcRects[charIndex].width + 1;
        slotVerts[9] = charDrawPos.y + fontDisplayInfo.chars.srcRects[charIndex].height;
        slotVerts[10] = charTexCoordsBottomRight.x;
        slotVerts[11] = charTexCoordsBottomRight.y;

        slotVerts[12] = charDrawPos.x;
        slotVerts[13] = charDrawPos.y + fontDisplayInfo.chars.srcRects[charIndex].height;
        slotVerts[14] = charTexCoordsTopLeft.x;
        slotVerts[15] = charTexCoordsBottomRight.y;
    }

    // Submit the vertex data.
    glBindVertexArray(batch.quadBufGLIDs.vertArrayGLID);
    glBindBuffer(GL_ARRAY_BUFFER, batch.quadBufGLIDs.vertBufGLID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertsLen * sizeof(verts[0]), verts);
}

void clear_char_batch(const Renderer &renderer, const CharBatchKey &key)
{
    const RenderLayer &layer = renderer.layers[key.layerIndex];
    const CharBatch &batch = layer.charBatches[key.batchIndex];

    glBindVertexArray(batch.quadBufGLIDs.vertArrayGLID);
    glBindBuffer(GL_ARRAY_BUFFER, batch.quadBufGLIDs.vertBufGLID);

    const int vertsLen = (gk_charBatchSlotVertsSize)*batch.slotCnt;
    const auto verts = cc::push_to_mem_arena<float>(g_tempMemArena, vertsLen);
    memset(verts, 0, vertsLen * sizeof(verts[0]));
    glBufferData(GL_ARRAY_BUFFER, vertsLen * sizeof(verts[0]), verts, GL_DYNAMIC_DRAW);
}
