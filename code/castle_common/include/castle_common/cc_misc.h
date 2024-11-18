#pragma once

#define CC_STATIC_ARRAY_LEN(X) (sizeof(X) / sizeof(X[0]))

namespace cc
{

using u_byte = unsigned char;

struct s_version
{
    int major;
    int minor;
    int patch;
};

}
