#include "c_audio.h"

#include "c_game.h"

static void load_music_buf_data(cc::MemArena &tempMemArena, const ALID bufALID, const MusicSrc &src, const AssetGroupManager &assetGroupManager)
{
    assert(bufALID);

    const auto buf = cc::push_to_mem_arena<cc::AudioSample>(tempMemArena, MusicSrc::k_bufSampleCnt);
    const int bytesRead = fread(buf, 1, MusicSrc::k_bufSize, src.fs);

    // TODO: Handle read failure.

    if (feof(src.fs))
    {
        // We've reached the end of the file, so rewind to the start for the next read.
        fseek(src.fs, 0, SEEK_SET);
    }

    const cc::AudioInfo &info = assetGroupManager.get_music_info(src.musicID);

    const ALenum format = info.channelCnt == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    alBufferData(bufALID, format, buf, bytesRead, info.sampleRate);
}

void SoundManager::clean()
{
    for (int i = 0; i < k_srcLimit; ++i)
    {
        if (m_alIDs[i])
        {
            alDeleteSources(1, &m_alIDs[i]);
        }
    }

    *this = SoundManager();
}

void SoundManager::handle_auto_release_srcs()
{
    for (int i = 0; i < k_srcLimit; ++i)
    {
        if (!is_bit_active(m_autoReleases, i))
        {
            continue;
        }

        ALint srcState;
        alGetSourcei(m_alIDs[i], AL_SOURCE_STATE, &srcState);

        if (srcState == AL_STOPPED)
        {
            release_src_by_index(i);
            deactivate_bit(m_autoReleases, i);
        }
    }
}

SoundSrcID SoundManager::add_src(const AssetID soundID, const AssetGroupManager &assetGroupManager)
{
    // TODO: To speed this up, consider storing and accessing the index of the most recently released source.
    for (int i = 0; i < k_srcLimit; ++i)
    {
        if (!m_alIDs[i])
        {
            alGenSources(1, &m_alIDs[i]);
            alSourcei(m_alIDs[i], AL_BUFFER, assetGroupManager.get_sound_buf_al_id(soundID));

            ++m_versions[i];

            return {
                .index = i,
                .version = m_versions[i]
            };
        }
    }

    assert(false);

    return {};
}

void SoundManager::remove_src(const SoundSrcID srcID)
{
    assert(srcID.index >= 0 && srcID.index < k_srcLimit);
    assert(m_versions[srcID.index] == srcID.version);
    assert(m_alIDs[srcID.index]);

    release_src_by_index(srcID.index);
}

void SoundManager::play_src(const SoundSrcID srcID, const AssetGroupManager &assetGroupManager, const float gain, const float pitch) const
{
    assert(srcID.index >= 0 && srcID.index < k_srcLimit);
    assert(m_versions[srcID.index] == srcID.version);
    assert(m_alIDs[srcID.index]);

    alSourceRewind(m_alIDs[srcID.index]); // Restart if already playing.
    alSourcef(m_alIDs[srcID.index], AL_GAIN, gain);
    alSourcef(m_alIDs[srcID.index], AL_PITCH, pitch);
    alSourcePlay(m_alIDs[srcID.index]);
}

void SoundManager::add_and_play_src(const AssetID soundID, const AssetGroupManager &assetGroupManager, const float gain, const float pitch)
{
    const SoundSrcID srcID = add_src(soundID, assetGroupManager);
    play_src(srcID, assetGroupManager, gain, pitch);
    activate_bit(m_autoReleases, srcID.index); // No reference to this source is returned, so it needs to be automatically released once it is detected as finished.
}

void SoundManager::release_src_by_index(const int index)
{
    assert(index >= 0 && index < k_srcLimit);
    assert(m_alIDs[index]);

    alDeleteSources(1, &m_alIDs[index]);
    m_alIDs[index] = 0;
}

void MusicManager::clean()
{
    for (int i = 0; i < k_srcLimit; ++i)
    {
        if (is_bit_active(m_activity, i))
        {
            clean_active_src(i);
        }
    }

    *this = {};
}

void MusicManager::refresh_src_bufs(cc::MemArena &tempMemArena, const AssetGroupManager &assetGroupManager) const
{
    for (int i = 0; i < k_srcLimit; ++i)
    {
        if (!is_bit_active(m_activity, i))
        {
            continue;
        }

        const MusicSrc &src = m_srcs[i];

        // Retrieve all processed buffers, fill them with new data and queue them again.
        int processedBufCnt;
        alGetSourcei(src.alID, AL_BUFFERS_PROCESSED, &processedBufCnt);

        while (processedBufCnt > 0)
        {
            ALID bufALID;
            alSourceUnqueueBuffers(src.alID, 1, &bufALID);

            load_music_buf_data(tempMemArena, bufALID, src, assetGroupManager);

            alSourceQueueBuffers(src.alID, 1, &bufALID);

            processedBufCnt--;
        }
    }
}

MusicSrcID MusicManager::add_src(const AssetID musicID, const AssetGroupManager &assetGroupManager)
{
    const int srcIndex = first_inactive_bit_index(m_activity);
    assert(srcIndex != -1);

    MusicSrc &src = m_srcs[srcIndex];
    src.musicID = musicID;

    alGenSources(1, &src.alID);
    alGenBuffers(MusicSrc::k_bufCnt, src.bufALIDs);

    activate_bit(m_activity, srcIndex);
    ++m_versions[srcIndex];

    return {
        .index = srcIndex,
        .version = m_versions[srcIndex]
    };
}

void MusicManager::remove_src(const MusicSrcID srcID)
{
    assert(srcID.index >= 0 && srcID.index < k_srcLimit);
    assert(m_versions[srcID.index] == srcID.version);
    assert(is_bit_active(m_activity, srcID.index));

    clean_active_src(srcID.index);
    deactivate_bit(m_activity, srcID.index);
}

void MusicManager::play_src(cc::MemArena &tempMemArena, const MusicSrcID id, const AssetGroupManager &assetGroupManager)
{
    assert(id.index >= 0 && id.index < k_srcLimit);
    assert(m_versions[id.index] == id.version);
    assert(is_bit_active(m_activity, id.index));

    MusicSrc &src = m_srcs[id.index];

    src.fs = fopen(assetGroupManager.get_music_filename(src.musicID), "rb");

    // TODO: Handle file open failure.

    for (int i = 0; i < MusicSrc::k_bufCnt; ++i)
    {
        load_music_buf_data(tempMemArena, src.bufALIDs[i], src, assetGroupManager);
    }

    alSourceQueueBuffers(src.alID, MusicSrc::k_bufCnt, src.bufALIDs);
    alSourcePlay(src.alID);
}

void MusicManager::clean_active_src(const int index)
{
    assert(is_bit_active(m_activity, index));

    alSourceStop(m_srcs[index].alID);
    alSourcei(m_srcs[index].alID, AL_BUFFER, 0);

    alDeleteBuffers(MusicSrc::k_bufCnt, m_srcs[index].bufALIDs);
    alDeleteSources(1, &m_srcs[index].alID);

    fclose(m_srcs[index].fs);

    m_srcs[index] = {};
}
