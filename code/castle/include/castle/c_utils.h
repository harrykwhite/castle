#pragma once

#include <glad/glad.h>
#include <castle_common/cc_debugging.h>
#include <castle_common/cc_misc.h>

using u_byte = unsigned char;
using u_gl_id = GLuint;

template<int BIT_CNT>
class c_bitset
{
public:
    void clear();
    int get_first_inactive_bit_index() const; // Returns -1 if not found.
    bool is_full() const;
    bool is_empty() const;

    inline bool is_bit_active(const int bit_index) const
    {
        CC_CHECK(bit_index >= 0 && bit_index < BIT_CNT);
        return m_bytes[bit_index / 8] & (static_cast<unsigned char>(1) << (bit_index % 8));
    }

    inline void activate_bit(const int bit_index)
    {
        CC_CHECK(bit_index >= 0 && bit_index < BIT_CNT);
        m_bytes[bit_index / 8] |= static_cast<unsigned char>(1) << (bit_index % 8);
    }

    inline void deactivate_bit(const int bit_index)
    {
        CC_CHECK(bit_index >= 0 && bit_index < BIT_CNT);
        m_bytes[bit_index / 8] &= ~(static_cast<unsigned char>(1) << (bit_index % 8));
    }

private:
    unsigned char m_bytes[(BIT_CNT + 7) & ~7] = {};
};

template<int BIT_CNT>
inline void c_bitset<BIT_CNT>::clear()
{
    for (int i = 0; i < CC_STATIC_ARRAY_LEN(m_bytes); ++i)
    {
        m_bytes[i] = 0x00;
    }
}

template<int BIT_CNT>
inline int c_bitset<BIT_CNT>::get_first_inactive_bit_index() const
{
    for (int i = 0; i < CC_STATIC_ARRAY_LEN(m_bytes); ++i)
    {
        if (m_bytes[i] == 0xFF)
        {
            continue;
        }

        for (int j = 0; j < 8; ++j)
        {
            if (!(m_bytes[i] & (1 << j)))
            {
                return (i * 8) + j;
            }
        }
    }

    return -1;
}

template<int BIT_CNT>
inline bool c_bitset<BIT_CNT>::is_full() const
{
    for (int i = 0; i < CC_STATIC_ARRAY_LEN(m_bytes); ++i)
    {
        if (m_bytes[i] != 0xFF)
        {
            return false;
        }
    }

    return true;
}

template<int BIT_CNT>
inline bool c_bitset<BIT_CNT>::is_empty() const
{
    for (int i = 0; i < CC_STATIC_ARRAY_LEN(m_bytes); ++i)
    {
        if (m_bytes[i] != 0x00)
        {
            return false;
        }
    }

    return true;
}
