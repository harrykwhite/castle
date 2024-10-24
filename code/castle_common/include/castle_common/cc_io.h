#pragma once

#include <cstdio>

namespace cc
{

template<typename T>
T read_next_item_from_file(std::FILE *const fs)
{
    T item;
    std::fread(&item, sizeof(item), 1, fs);
    return item;
}

}
