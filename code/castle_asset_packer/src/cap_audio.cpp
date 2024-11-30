#include <AudioFile.h>
#include "cap_shared.h"

static const char *const ik_soundFilePathEnds[] = {
    "\\sounds\\bloop.wav"
};

static_assert(cc::VANILLA_SOUND_CNT == CC_STATIC_ARRAY_LEN(ik_soundFilePathEnds));

static const char *const ik_musicFilePathEnds[] = {
    "\\music\\beat.wav"
};

static_assert(cc::VANILLA_MUSIC_CNT == CC_STATIC_ARRAY_LEN(ik_musicFilePathEnds));

static bool load_audio_file_and_write_data(const char *const filePath, FILE *const infoFS, FILE *const samplesFS)
{
    // Open the audio file.
    AudioFile<cc::AudioSample> audioFile;
    audioFile.shouldLogErrorsToConsole(false);

    if (!audioFile.load(filePath))
    {
        cc::log_error("Failed to load audio file \"%s\".", filePath);
        return false;
    }

    // Write audio file information.
    const cc::AudioInfo info = {
        .channelCnt = audioFile.getNumChannels(),
        .sampleCntPerChannel = audioFile.getNumSamplesPerChannel(),
        .sampleRate = audioFile.getSampleRate()
    };

    fwrite(&info, sizeof(info), 1, infoFS);

    // Write the sample data.
    if (info.channelCnt == 1)
    {
        for (int i = 0; i < info.sampleCntPerChannel; ++i)
        {
            const cc::AudioSample sample = audioFile.samples[0][i];
            fwrite(&sample, sizeof(sample), 1, samplesFS);
        }
    }
    else
    {
        // The channel count is 2.
        for (int i = 0; i < info.sampleCntPerChannel; ++i)
        {
            const cc::AudioSample frame[2] = {
                audioFile.samples[0][i],
                audioFile.samples[1][i]
            };

            fwrite(frame, sizeof(frame), 1, samplesFS);
        }
    }

    return true;
}

bool pack_sounds(FILE *const assetFileStream, const char *const assetsDir, cc::MemArena &memArena)
{
    for (const char *const soundFilePathEnd : ik_soundFilePathEnds)
    {
        char soundFilePath[gk_assetFilePathMaxLen + 1];
        snprintf(soundFilePath, sizeof(soundFilePath), "%s%s", assetsDir, soundFilePathEnd);

        if (!load_audio_file_and_write_data(soundFilePath, assetFileStream, assetFileStream))
        {
            return false;
        }

        cc::log("Successfully packed sound with file path \"%s\".", soundFilePath);
    }

    return true;
}

bool pack_music(FILE *const assetFileStream, const char *const assetsDir, cc::MemArena &memArena)
{
    for (const char *const musicFilePathEnd : ik_musicFilePathEnds)
    {
        // Determine the music file path.
        char musicFilePath[gk_assetFilePathMaxLen + 1];
        snprintf(musicFilePath, sizeof(musicFilePath), "%s%s", assetsDir, musicFilePathEnd);

        // Determine the output music file name and path.
        char outputMusicFileName[256];
        const int outputMusicFileNameStemLen = cc::extract_filename_from_path_no_ext(musicFilePath, outputMusicFileName, sizeof(outputMusicFileName));
        strncat(outputMusicFileName, ".dat", sizeof(outputMusicFileName) - outputMusicFileNameStemLen - 1);

        // Check and write the length of the output music file name.
        const int outputMusicFileNameLen = outputMusicFileNameStemLen + 4; // Account for the ".dat" extension.

        if (outputMusicFileNameLen > 255)
        {
            cc::log_error("Output music file name \"%s\" exceeds the length limit of 255 characters!", outputMusicFileName);
            return false;
        }

        const unsigned char outputMusicFileNameLenUC = outputMusicFileNameLen;
        fwrite(&outputMusicFileNameLenUC, sizeof(outputMusicFileNameLenUC), 1, assetFileStream); // First byte is the string length.

        // Write the output music file name.
        fwrite(outputMusicFileName, sizeof(*outputMusicFileName), outputMusicFileNameLen, assetFileStream); // The rest of the bytes are the characters without a terminating '\0'.

        // Open the output music file.
        FILE *const outputMusicFileStream = fopen(outputMusicFileName, "wb");

        if (!outputMusicFileStream)
        {
            cc::log_error("Failed to create or replace \"%s\" from music file \"%s\".", outputMusicFileName, musicFilePath);
            return false;
        }

        // Load the music file and write its information to the input assets file and samples to the output music file.
        if (!load_audio_file_and_write_data(musicFilePath, assetFileStream, outputMusicFileStream))
        {
            return false;
        }

        fclose(outputMusicFileStream);

        cc::log("Successfully packed music with file path \"%s\".", musicFilePath);
    }

    return true;
}
