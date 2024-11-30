#pragma once

#include <stdio.h>
#include <castle_common/cc_debugging.h>
#include <castle_common/cc_math.h>
#include <castle_common/cc_assets.h>
#include <castle_common/cc_misc.h>
#include <castle_common/cc_mem.h>
#include <castle_common/cc_io.h>

constexpr int gk_assetFilePathMaxLen = 255;

bool pack_textures(FILE *const assetFileStream, const char *const assetsDir);
bool pack_fonts(FILE *const assetFileStream, const char *const assetsDir, cc::MemArena &memArena);
bool pack_sounds(FILE *const assetFileStream, const char *const assetsDir, cc::MemArena &memArena);
bool pack_music(FILE *const assetFileStream, const char *const assetsDir, cc::MemArena &memArena);
