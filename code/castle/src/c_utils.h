#pragma once

#include <assert.h>
#include <glad/glad.h>
#include <AL/al.h>
#include <castle_common/cc_misc.h>
#include <castle_common/cc_mem.h>

#define LOCAL_PERSIST static

using GLID = GLuint;
using ALID = ALuint;

enum Sign
{
    SIGN_NEG = -1,
    SIGN_ZERO = 0,
    SIGN_POS = 1
};

constexpr int bits_to_bytes(const int bitCnt)
{
    return ((bitCnt + 7) & ~7) / 8;
}

constexpr int bytes_to_bits(const int byteCnt)
{
    return byteCnt * 8;
}

constexpr int kilobytes_to_bytes(const int kb)
{
    return kb << 10;
}

constexpr int megabytes_to_bytes(const int mb)
{
    return mb << 20;
}

constexpr int gigabytes_to_bytes(const int gb)
{
    return gb << 30;
}

template<int Bits>
struct StaticBitset
{
    static_assert(Bits > 0);

    cc::Byte bytes[bits_to_bytes(Bits)];
};

struct Bitset
{
    cc::Byte *bytes;
    int bitCnt;
};

inline bool is_bit_active(const cc::Byte *const bytes, const int bitIndex)
{
    assert(bitIndex >= 0);
    return bytes[bitIndex / 8] & (1 << (bitIndex % 8));
}

template<int Bits>
inline bool is_bit_active(const StaticBitset<Bits> &bitset, const int bitIndex)
{
    assert(bitIndex < Bits);
    return is_bit_active(bitset.bytes, bitIndex);
}

inline bool is_bit_active(const Bitset &bitset, const int bitIndex)
{
    assert(bitIndex < bitset.bitCnt);
    return is_bit_active(bitset.bytes, bitIndex);
}

bool are_all_bits_active(const cc::Byte *const bytes, const int bitCnt);

template<int Bits>
inline bool are_all_bits_active(const StaticBitset<Bits> &bitset)
{
    return are_all_bits_active(bitset.bytes, Bits);
}

inline bool are_all_bits_active(const Bitset &bitset)
{
    return are_all_bits_active(bitset.bytes, bitset.bitCnt);
}

inline void activate_bit(cc::Byte *const bytes, const int bitIndex)
{
    assert(bitIndex >= 0);
    bytes[bitIndex / 8] |= 1 << (bitIndex % 8);
}

template<int Bits>
inline void activate_bit(StaticBitset<Bits> &bitset, const int bitIndex)
{
    assert(bitIndex < Bits);
    activate_bit(bitset.bytes, bitIndex);
}

inline void activate_bit(Bitset &bitset, const int bitIndex)
{
    assert(bitIndex < bitset.bitCnt);
    activate_bit(bitset.bytes, bitIndex);
}

inline void deactivate_bit(cc::Byte *const bytes, const int bitIndex)
{
    assert(bitIndex >= 0);
    bytes[bitIndex / 8] &= ~(1 << (bitIndex % 8));
}

template<int Bits>
inline void deactivate_bit(StaticBitset<Bits> &bitset, const int bitIndex)
{
    assert(bitIndex < Bits);
    deactivate_bit(bitset.bytes, bitIndex);
}

inline void deactivate_bit(Bitset &bitset, const int bitIndex)
{
    assert(bitIndex < bitset.bitCnt);
    deactivate_bit(bitset.bytes, bitIndex);
}

inline void clear_bits(cc::Byte *const bytes, const int bitCnt)
{
    assert(bitCnt > 0);
    memset(bytes, 0, bits_to_bytes(bitCnt));
}

template<int Bits>
inline void clear_bits(StaticBitset<Bits> &bitset)
{
    clear_bits(bitset.bytes, Bits);
}

inline void clear_bits(Bitset &bitset)
{
    clear_bits(bitset.bytes, bitset.bitCnt);
}

int first_active_bit_index(const cc::Byte *const bytes, const int bitCnt);

template<int Bits>
inline int first_active_bit_index(const StaticBitset<Bits> &bitset)
{
    return first_active_bit_index(bitset.bytes, Bits);
}

inline int first_active_bit_index(const Bitset &bitset)
{
    return first_active_bit_index(bitset.bytes, bitset.bitCnt);
}

int first_inactive_bit_index(const cc::Byte *const bytes, const int bitCnt);

template<int Bits>
inline int first_inactive_bit_index(const StaticBitset<Bits> &bitset)
{
    return first_inactive_bit_index(bitset.bytes, Bits);
}

inline int first_inactive_bit_index(const Bitset &bitset)
{
    return first_inactive_bit_index(bitset.bytes, bitset.bitCnt);
}
