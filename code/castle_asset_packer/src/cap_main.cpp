#include "cap_shared.h"

constexpr int ik_memArenaSize = (1 << 20) * 64;

int main(const int argCnt, const char *const *const args)
{
    if (argCnt != 3)
    {
        cc::log_error("An assets directory and an output assets file path must both be provided as command-line arguments!");
        return EXIT_FAILURE;
    }

    const char *const assetsDir = args[1];
    const char *const assetsFilePath = args[2];

    // Ensure the assets file name is correct.
    const char *const assetsFileName = cc::extract_filename_from_path(assetsFilePath);

    if (strcmp(cc::gk_assetsFileName, assetsFileName))
    {
        cc::log_error("The output assets file must be named \"%s\"!", cc::gk_assetsFileName);
        return EXIT_FAILURE;
    }

    // Open the assets file.
    FILE *const assetsFileStream = fopen(assetsFilePath, "wb");

    if (!assetsFileStream)
    {
        cc::log_error("Failed to create or replace assets file with path \"%s\"!", assetsFilePath);
        return EXIT_FAILURE;
    }

    // Write asset counts to the file header.
    const int texCnt = cc::CORE_TEX_CNT;
    fwrite(&texCnt, sizeof(texCnt), 1, assetsFileStream);

    const int fontCnt = cc::CORE_FONT_CNT;
    fwrite(&fontCnt, sizeof(fontCnt), 1, assetsFileStream);

    const int sndCnt = cc::CORE_SOUND_CNT;
    fwrite(&sndCnt, sizeof(sndCnt), 1, assetsFileStream);

    const int musicCnt = cc::CORE_MUSIC_CNT;
    fwrite(&musicCnt, sizeof(musicCnt), 1, assetsFileStream);

    // Create the memory arena.
    cc::MemArena memArena = {};
    cc::init_mem_arena(memArena, ik_memArenaSize);

    // Pack assets.
    const bool packingSuccessful = pack_textures(assetsFileStream, assetsDir)
        && pack_fonts(assetsFileStream, assetsDir, memArena)
        && pack_sounds(assetsFileStream, assetsDir, memArena)
        && pack_music(assetsFileStream, assetsDir, memArena);

    cc::clean_mem_arena(memArena);
    fclose(assetsFileStream);

    if (!packingSuccessful)
    {
        remove(assetsFilePath);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
