#pragma once

#include <stdio.h>
#include "c_assets.h"
#include "c_utils.h"

struct SoundSrcID
{
    int index;
    int version;
};

class SoundManager
{
public:
    void clean();

    void handle_auto_release_srcs();
    SoundSrcID add_src(const AssetID soundID, const AssetGroupManager &assetGroupManager);
    void remove_src(const SoundSrcID srcID);
    void play_src(const SoundSrcID srcID, const AssetGroupManager &assetGroupManager, const float gain = 1.0f, const float pitch = 1.0f) const;
    void add_and_play_src(const AssetID soundID, const AssetGroupManager &assetGroupManager, const float gain = 1.0f, const float pitch = 1.0f);

private:
    static constexpr int k_srcLimit = 128;

    ALID m_alIDs[k_srcLimit];
    int m_versions[k_srcLimit];
    StaticBitset<k_srcLimit> m_autoReleases; // Indicates which sources need to be automatically released when finished (due to not them not being referenced).

    void release_src_by_index(const int index);
};

struct MusicSrcID
{
    int index;
    int version;
};

struct MusicSrc
{
    static constexpr int k_bufCnt = 4;
    static constexpr int k_bufSampleCnt = 44100;
    static constexpr int k_bufSize = k_bufSampleCnt * sizeof(cc::AudioSample);

    AssetID musicID;

    ALID alID;
    ALID bufALIDs[k_bufCnt];

    FILE *fs;
};

class MusicManager
{
public:
    void clean();

    void refresh_src_bufs(cc::MemArena &tempMemArena, const AssetGroupManager &assetGroupManager) const;
    MusicSrcID add_src(const AssetID musicID, const AssetGroupManager &assetGroupManager);
    void remove_src(const MusicSrcID srcID);
    void play_src(cc::MemArena &tempMemArena, const MusicSrcID id, const AssetGroupManager &assetGroupManager);

private:
    static constexpr int k_srcLimit = 16;

    MusicSrc m_srcs[k_srcLimit];
    StaticBitset<k_srcLimit> m_activity;
    int m_versions[k_srcLimit];

    void clean_active_src(const int index);
};
