#pragma once

#include <fstream>

namespace cc
{

template<typename T>
inline T read_from_ifs(std::ifstream &ifs)
{
    T val;
    ifs.read(reinterpret_cast<char *>(&val), sizeof(val));
    return val;
}

}
