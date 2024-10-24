#pragma once

#define CC_STATIC_ARRAY_LEN(X) (sizeof(X) / sizeof(X[0]))

namespace cc
{

// Returns the number of bits a given unsigned integer uses. For example, an input of 5 will return 3, and an input of 0 will return 1.
constexpr int get_uint_bit_cnt(const unsigned int n)
{
    return n > 1 ? 1 + get_uint_bit_cnt(n >> 1) : 1;
}

// Copies a source string of a maximum length (excluding the null terminator) to a destination. Returns -1 if a null terminator in the source string was not reached before hitting the max length.
int copy_str_and_get_len(const char *const src, char *const dest, const int max_len);

}
