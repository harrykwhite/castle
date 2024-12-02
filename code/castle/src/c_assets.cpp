#include "c_assets.h"

#include <stdio.h>
#include <assert.h>
#include <castle_common/cc_io.h>
#include <castle_common/cc_debugging.h>
#include "c_game.h"

static const char *const ik_spriteQuadVertShaderSrc = R"(#version 430 core

layout (location = 0) in vec2 a_vert;
layout (location = 1) in vec2 a_pos;
layout (location = 2) in vec2 a_size;
layout (location = 3) in float a_rot;
layout (location = 4) in float a_texIndex;
layout (location = 5) in vec2 a_texCoord;
layout (location = 6) in float a_alpha;

out flat int v_texIndex;
out vec2 v_texCoord;
out float v_alpha;

uniform mat4 u_view;
uniform mat4 u_proj;

void main()
{
    float rotCos = cos(a_rot);
    float rotSin = -sin(a_rot);

    mat4 model = mat4(
        vec4(a_size.x * rotCos, a_size.x * rotSin, 0.0f, 0.0f),
        vec4(a_size.y * -rotSin, a_size.y * rotCos, 0.0f, 0.0f),
        vec4(0.0f, 0.0f, 1.0f, 0.0f),
        vec4(a_pos.x, a_pos.y, 0.0f, 1.0f)
    );

    gl_Position = u_proj * u_view * model * vec4(a_vert, 0.0f, 1.0f);

    v_texIndex = int(a_texIndex);
    v_texCoord = a_texCoord;
    v_alpha = a_alpha;
}
)";

static const char *const ik_spriteQuadFragShaderSrc = R"(#version 430 core

in flat int v_texIndex;
in vec2 v_texCoord;
in float v_alpha;

out vec4 o_fragColor;

uniform sampler2D u_textures[32];

void main()
{
    vec4 texColor = texture(u_textures[v_texIndex], v_texCoord);
    o_fragColor = texColor * vec4(1.0f, 1.0f, 1.0f, v_alpha);
}
)";

static const char *const ik_charQuadVertShaderSrc = R"(#version 430 core

layout (location = 0) in vec2 a_vert;
layout (location = 1) in vec2 a_texCoord;

out vec2 v_texCoord;

uniform vec2 u_pos;
uniform float u_rot;

uniform mat4 u_proj;
uniform mat4 u_view;

void main()
{
    float rotCos = cos(u_rot);
    float rotSin = sin(u_rot);

    mat4 model = mat4(
        vec4(rotCos, rotSin, 0.0f, 0.0f),
        vec4(-rotSin, rotCos, 0.0f, 0.0f),
        vec4(0.0f, 0.0f, 1.0f, 0.0f),
        vec4(u_pos.x, u_pos.y, 0.0f, 1.0f)
    );

    gl_Position = u_proj * u_view * model * vec4(a_vert, 0.0f, 1.0f);

    v_texCoord = a_texCoord;
}
)";

static const char *const ik_charQuadFragShaderSrc = R"(#version 430 core

in vec2 v_texCoord;

out vec4 o_fragColor;

uniform vec4 u_blend;
uniform sampler2D u_tex;

void main()
{
    vec4 texColor = texture(u_tex, v_texCoord);
    o_fragColor = texColor * u_blend;
}
)";

static GLID create_shader_prog_from_srcs(const char *const vertShaderSrc, const char *const fragShaderSrc)
{
    const GLID vertShaderGLID = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShaderGLID, 1, &vertShaderSrc, nullptr);
    glCompileShader(vertShaderGLID);

    const GLID fragShaderGLID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShaderGLID, 1, &fragShaderSrc, nullptr);
    glCompileShader(fragShaderGLID);

    // TODO: Check for shader compilation errors, and return 0 if there are any.

    const GLID progGLID = glCreateProgram();
    glAttachShader(progGLID, vertShaderGLID);
    glAttachShader(progGLID, fragShaderGLID);
    glLinkProgram(progGLID);

    glDeleteShader(vertShaderGLID);
    glDeleteShader(fragShaderGLID);

    return progGLID;
}

static void init_textures_with_fs(Textures &textures, FILE *const fs, cc::MemArena &tempMemArena, const int texCnt)
{
    assert(texCnt >= 0);

    if (!texCnt)
    {
        return;
    }

    // Generate textures and store their IDs.
    glGenTextures(texCnt, textures.glIDs);

    // Read the sizes and pixel data of textures and finish setting them up.
    const int pxDataBufSize = cc::gk_texChannelCnt * cc::gk_texSizeLimit.x * cc::gk_texSizeLimit.y;
    const auto pxDataBuf = cc::push_to_mem_arena<unsigned char>(tempMemArena, pxDataBufSize); // Working space for temporarily storing the pixel data of each texture.

    for (int i = 0; i < texCnt; ++i)
    {
        textures.sizes[i] = cc::read_from_fs<cc::Vec2DInt>(fs);

        fread(pxDataBuf, 1, cc::gk_texChannelCnt * textures.sizes[i].x * textures.sizes[i].y, fs);

        glBindTexture(GL_TEXTURE_2D, textures.glIDs[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textures.sizes[i].x, textures.sizes[i].y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pxDataBuf);
    }
}

static void init_fonts_with_fs(Fonts &fonts, FILE *const fs, cc::MemArena &tempMemArena, const int fontCnt)
{
    assert(fontCnt >= 0);

    if (!fontCnt)
    {
        return;
    }

    // Generate textures and store their IDs.
    glGenTextures(fontCnt, fonts.texGLIDs);

    // Read the sizes and pixel data of textures and finish setting them up.
    const int pxDataBufSize = cc::gk_texChannelCnt * cc::gk_texSizeLimit.x * cc::gk_texSizeLimit.y;
    const auto pxDataBuf = cc::push_to_mem_arena<unsigned char>(tempMemArena, pxDataBufSize); // Working space for temporarily storing the pixel data of each font texture.

    for (int i = 0; i < fontCnt; ++i)
    {
        fread(&fonts.displayInfos[i], sizeof(fonts.displayInfos[i]), 1, fs);

        fread(pxDataBuf, 1, pxDataBufSize, fs);

        glBindTexture(GL_TEXTURE_2D, fonts.texGLIDs[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fonts.displayInfos[i].texSize.x, fonts.displayInfos[i].texSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pxDataBuf);
    }
}

static void init_sounds_with_fs(Sounds &sounds, FILE *const fs, cc::MemArena &tempMemArena, const int soundCnt)
{
    assert(soundCnt >= 0);

    if (!soundCnt)
    {
        return;
    }

    alGenBuffers(soundCnt, sounds.bufALIDs);

    for (int i = 0; i < soundCnt; ++i)
    {
        const auto audioInfo = cc::read_from_fs<cc::AudioInfo>(fs);

        const int sampleCnt = audioInfo.sampleCntPerChannel * audioInfo.channelCnt;
        const auto sampleData = cc::push_to_mem_arena<cc::AudioSample>(tempMemArena, sampleCnt);
        const int sampleDataSize = sampleCnt * sizeof(cc::AudioSample);

        fread(sampleData, sizeof(*sampleData), sampleCnt, fs);

        const ALenum format = audioInfo.channelCnt == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
        alBufferData(sounds.bufALIDs[i], format, sampleData, sampleDataSize, audioInfo.sampleRate);
    }
}

static void init_music_with_fs(Music &music, FILE *const fs, const int musicCnt)
{
    assert(musicCnt >= 0);

    if (!musicCnt)
    {
        return;
    }

    for (int i = 0; i < musicCnt; ++i)
    {
        const auto filenameLen = cc::read_from_fs<cc::Byte>(fs);

        fread(music.filenames[i], 1, filenameLen, fs);
        music.filenames[i][filenameLen] = '\0';

        music.infos[i] = cc::read_from_fs<cc::AudioInfo>(fs);
    }
}

bool load_shader_progs(ShaderProgs &progs)
{
    // Load the sprite quad shader program.
    progs.spriteQuadGLID = create_shader_prog_from_srcs(ik_spriteQuadVertShaderSrc, ik_spriteQuadFragShaderSrc);

    if (!progs.spriteQuadGLID)
    {
        return false;
    }

    progs.spriteQuadProjUniLoc = glGetUniformLocation(progs.spriteQuadGLID, "u_proj");
    progs.spriteQuadViewUniLoc = glGetUniformLocation(progs.spriteQuadGLID, "u_view");
    progs.spriteQuadTexturesUniLoc = glGetUniformLocation(progs.spriteQuadGLID, "u_textures");

    // Load the character quad shader program.
    progs.charQuadGLID = create_shader_prog_from_srcs(ik_charQuadVertShaderSrc, ik_charQuadFragShaderSrc);

    if (!progs.charQuadGLID)
    {
        glDeleteProgram(progs.spriteQuadGLID);
        progs = {};
        return false;
    }

    progs.charQuadProjUniLoc = glGetUniformLocation(progs.charQuadGLID, "u_proj");
    progs.charQuadViewUniLoc = glGetUniformLocation(progs.charQuadGLID, "u_view");
    progs.charQuadPosUniLoc = glGetUniformLocation(progs.charQuadGLID, "u_pos");
    progs.charQuadRotUniLoc = glGetUniformLocation(progs.charQuadGLID, "u_rot");
    progs.charQuadBlendUniLoc = glGetUniformLocation(progs.charQuadGLID, "u_blend");

    return true;
}

void clean_shader_progs(ShaderProgs &progs)
{
    glDeleteProgram(progs.spriteQuadGLID);
    glDeleteProgram(progs.charQuadGLID);

    progs = {};
}

bool AssetGroupManager::init(cc::MemArena &permMemArena, cc::MemArena &tempMemArena)
{
    m_groups = cc::push_to_mem_arena<AssetGroup>(permMemArena, k_groupLimit);
    return init_core_group(tempMemArena);
}

void AssetGroupManager::clean()
{
    for (int i = 0; i < k_groupLimit; ++i)
    {
        if (is_bit_active(m_groupActivity, i))
        {
            clean_asset_group(i);
        }
    }
}

bool AssetGroupManager::init_core_group(cc::MemArena &tempMemArena)
{
    assert(!m_groupVersions[0]);

    // Try opening the assets file.
    FILE *const fs = fopen(cc::gk_assetsFileName, "rb");

    if (!fs)
    {
        cc::log_error("Failed to open \"%s\"!", cc::gk_assetsFileName);
        return false;
    }

    // Read asset counts from the header.
    m_groups[0].texCnt = cc::read_from_fs<int>(fs);
    m_groups[0].fontCnt = cc::read_from_fs<int>(fs);
    m_groups[0].soundCnt = cc::read_from_fs<int>(fs);
    m_groups[0].musicCnt = cc::read_from_fs<int>(fs);

    // Load asset data.
    init_textures_with_fs(m_groups[0].textures, fs, tempMemArena, m_groups[0].texCnt);
    init_fonts_with_fs(m_groups[0].fonts, fs, tempMemArena, m_groups[0].fontCnt);
    init_sounds_with_fs(m_groups[0].sounds, fs, tempMemArena, m_groups[0].soundCnt);
    init_music_with_fs(m_groups[0].music, fs, m_groups[0].musicCnt);

    // Update the group version.
    ++m_groupVersions[0];

    // Mark the core group as active.
    activate_bit(m_groupActivity, 0);

    // Close the file stream.
    fclose(fs);

    return true;
}

void AssetGroupManager::clean_asset_group(const int index)
{
    AssetGroup &group = m_groups[index];

    alDeleteBuffers(group.soundCnt, group.sounds.bufALIDs);
    glDeleteTextures(group.fontCnt, group.fonts.texGLIDs);
    glDeleteTextures(group.texCnt, group.textures.glIDs);

    memset(&group, 0, sizeof(group));
}
