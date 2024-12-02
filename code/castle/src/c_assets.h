#pragma once

#include <castle_common/cc_math.h>
#include <castle_common/cc_assets.h>
#include <castle_common/cc_mem.h>
#include "c_utils.h"
#include "c_modding.h"

constexpr int gk_spriteQuadShaderProgVertCnt = 11;
constexpr int gk_charQuadShaderProgVertCnt = 4;

// NOTE: If there is a fixed limit on the number of assets in a mod, then the asset ID can be a single integer.
struct AssetID
{
    int groupIndex;
    int groupVersion;

    int index;

    bool operator==(const AssetID &other) const
    {
        return groupIndex == other.groupIndex && groupVersion == other.groupVersion && index == other.index;
    }
};

struct Textures
{
    static constexpr int k_limit = 256;

    GLID glIDs[k_limit];
    cc::Vec2DInt sizes[k_limit];
};

struct Fonts
{
    static constexpr int k_limit = 32;

    GLID texGLIDs[k_limit];
    cc::FontDisplayInfo displayInfos[k_limit];
};

struct Sounds
{
    static constexpr int k_limit = 256;

    ALID bufALIDs[k_limit];
};

struct Music
{
    static constexpr int k_limit = 32;

    char filenames[k_limit][cc::gk_musicFileNameMaxLen + 1];
    cc::AudioInfo infos[k_limit];
};

struct AssetGroup
{
    int texCnt;
    int fontCnt;
    int soundCnt;
    int musicCnt;

    Textures textures;
    Fonts fonts;
    Sounds sounds;
    Music music;
};

struct ShaderProgs
{
    GLID spriteQuadGLID;
    int spriteQuadProjUniLoc;
    int spriteQuadViewUniLoc;
    int spriteQuadTexturesUniLoc;

    GLID charQuadGLID;
    int charQuadProjUniLoc;
    int charQuadViewUniLoc;
    int charQuadPosUniLoc;
    int charQuadRotUniLoc;
    int charQuadBlendUniLoc;
};

class AssetGroupManager
{
public:
    static constexpr int k_groupLimit = 1 + k_modLimit;

    bool init(cc::MemArena &permMemArena, cc::MemArena &tempMemArena);
    void clean();

    inline GLID get_tex_gl_id(const AssetID &id) const
    {
        asset_id_asserts(id, m_groups[id.groupIndex].texCnt);
        return m_groups[id.groupIndex].textures.glIDs[id.index];
    }

    inline cc::Vec2DInt get_tex_size(const AssetID &id) const
    {
        asset_id_asserts(id, m_groups[id.groupIndex].texCnt);
        return m_groups[id.groupIndex].textures.sizes[id.index];
    }

    inline GLID get_font_tex_gl_id(const AssetID &id) const
    {
        asset_id_asserts(id, m_groups[id.groupIndex].fontCnt);
        return m_groups[id.groupIndex].fonts.texGLIDs[id.index];
    }

    inline const cc::FontDisplayInfo &get_font_display_info(const AssetID &id) const
    {
        asset_id_asserts(id, m_groups[id.groupIndex].fontCnt);
        return m_groups[id.groupIndex].fonts.displayInfos[id.index];
    }

    inline ALID get_sound_buf_al_id(const AssetID &id) const
    {
        asset_id_asserts(id, m_groups[id.groupIndex].soundCnt);
        return m_groups[id.groupIndex].sounds.bufALIDs[id.index];
    }

    inline const char *get_music_filename(const AssetID &id) const
    {
        asset_id_asserts(id, m_groups[id.groupIndex].musicCnt);
        return m_groups[id.groupIndex].music.filenames[id.index];
    }

    inline const cc::AudioInfo &get_music_info(const AssetID &id) const
    {
        asset_id_asserts(id, m_groups[id.groupIndex].musicCnt);
        return m_groups[id.groupIndex].music.infos[id.index];
    }

private:
    AssetGroup *m_groups;
    int m_groupVersions[k_groupLimit];
    StaticBitset<k_groupLimit> m_groupActivity;

    bool init_core_group(cc::MemArena &tempMemArena);
    void clean_asset_group(const int index);

    inline void asset_id_asserts(const AssetID &id, const int assetCnt) const
    {
        assert(id.groupIndex >= 0 && id.groupIndex < k_groupLimit);
        assert(id.groupVersion == m_groupVersions[id.groupIndex]);
        assert(id.index >= 0 && id.index < assetCnt);
    }
};

bool load_shader_progs(ShaderProgs &progs);
void clean_shader_progs(ShaderProgs &progs);

constexpr AssetID make_core_asset_id(const int index)
{
    return {
        .groupIndex = 0,
        .groupVersion = 1,
        .index = index
    };
}
