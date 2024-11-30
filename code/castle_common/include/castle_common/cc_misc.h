#pragma once

#define CC_STATIC_ARRAY_LEN(X) (sizeof(X) / sizeof(X[0]))

namespace cc
{

using Byte = unsigned char;

struct Version
{
    int major;
    int minor;
    int patch;
};

}
