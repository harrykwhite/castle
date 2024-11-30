#pragma once

#include <algorithm>
#include <stdio.h>
#include <string.h>

namespace cc
{

const char *extract_filename_from_path(const char *const filePath);
int extract_filename_from_path_no_ext(const char *const filePath, char *const dest, const int destSize);

template<typename T>
inline T read_from_fs(FILE *const fs)
{
    T val;
    fread(&val, sizeof(val), 1, fs);
    return val;
}

}
