#pragma once

#include <assert.h>
#include <glad/glad.h>
#include <AL/al.h>
#include <castle_common/cc_misc.h>
#include <castle_common/cc_mem.h>

using GLID = GLuint;
using ALID = ALuint;

constexpr int bit_to_byte_cnt(const int bitCnt)
{
    return (bitCnt + 7) & ~7;
}

template<int Bits>
struct Bitset
{
    static_assert(Bits > 0);

    cc::Byte bytes[bit_to_byte_cnt(Bits)];
};

struct HeapBitset
{
    cc::Byte *bytes;
    int bitCnt;
};

inline void init_heap_bitset(HeapBitset &bitset, cc::MemArena &memArena, const int bitCnt)
{
    assert(bitCnt > 0);

    bitset.bytes = cc::push_to_mem_arena<cc::Byte>(memArena, bit_to_byte_cnt(bitCnt));
    bitset.bitCnt = bitCnt;
}

inline bool is_bit_active(const cc::Byte *const bytes, const int bitIndex)
{
    assert(bitIndex >= 0);
    return bytes[bitIndex / 8] & (1 << (bitIndex % 8));
}

template<int Bits>
inline bool is_bit_active(const Bitset<Bits> &bitset, const int bitIndex)
{
    assert(bitIndex < Bits);
    return is_bit_active(bitset.bytes, bitIndex);
}

inline bool is_bit_active(const HeapBitset &bitset, const int bitIndex)
{
    assert(bitIndex < bitset.bitCnt);
    return is_bit_active(bitset.bytes, bitIndex);
}

bool are_all_bits_active(const cc::Byte *const bytes, const int bitCnt);

template<int Bits>
inline bool are_all_bits_active(const Bitset<Bits> &bitset)
{
    return are_all_bits_active(bitset.bytes, Bits);
}

inline bool are_all_bits_active(const HeapBitset &bitset)
{
    return are_all_bits_active(bitset.bytes, bitset.bitCnt);
}

inline void activate_bit(cc::Byte *const bytes, const int bitIndex)
{
    assert(bitIndex >= 0);
    bytes[bitIndex / 8] |= 1 << (bitIndex % 8);
}

template<int Bits>
inline void activate_bit(Bitset<Bits> &bitset, const int bitIndex)
{
    assert(bitIndex < Bits);
    activate_bit(bitset.bytes, bitIndex);
}

inline void activate_bit(HeapBitset &bitset, const int bitIndex)
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
inline void deactivate_bit(Bitset<Bits> &bitset, const int bitIndex)
{
    assert(bitIndex < Bits);
    deactivate_bit(bitset.bytes, bitIndex);
}

inline void deactivate_bit(HeapBitset &bitset, const int bitIndex)
{
    assert(bitIndex < bitset.bitCnt);
    deactivate_bit(bitset.bytes, bitIndex);
}

inline void clear_bits(cc::Byte *const bytes, const int bitCnt)
{
    assert(bitCnt > 0);
    memset(bytes, 0, bit_to_byte_cnt(bitCnt));
}

template<int Bits>
inline void clear_bits(Bitset<Bits> &bitset)
{
    clear_bits(bitset.bytes, Bits);
}

inline void clear_bits(HeapBitset &bitset)
{
    clear_bits(bitset.bytes, bitset.bitCnt);
}

int first_active_bit_index(const cc::Byte *const bytes, const int bitCnt);

template<int Bits>
inline int first_active_bit_index(const Bitset<Bits> &bitset)
{
    return first_active_bit_index(bitset.bytes, Bits);
}

inline int first_active_bit_index(const HeapBitset &bitset)
{
    return first_active_bit_index(bitset.bytes, bitset.bitCnt);
}

int first_inactive_bit_index(const cc::Byte *const bytes, const int bitCnt);

template<int Bits>
inline int first_inactive_bit_index(const Bitset<Bits> &bitset)
{
    return first_inactive_bit_index(bitset.bytes, Bits);
}

inline int first_inactive_bit_index(const HeapBitset &bitset)
{
    return first_inactive_bit_index(bitset.bytes, bitset.bitCnt);
}
