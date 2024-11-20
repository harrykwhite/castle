#pragma once

#include <cassert>
#include <memory>
#include <glad/glad.h>
#include <castle_common/cc_debugging.h>
#include <castle_common/cc_misc.h>

using u_gl_id = GLuint;

class c_heap_bitset
{
public:
    int get_first_inactive_bit_index() const; // Returns -1 if all bits are active.
    bool is_full() const;
    bool is_clear() const;

    c_heap_bitset(const int byte_cnt) : m_bytes(std::make_unique<cc::u_byte[]>(byte_cnt)), m_byte_cnt(byte_cnt)
    {
    }

    inline void fill()
    {
        std::fill(m_bytes.get(), m_bytes.get() + m_byte_cnt, 0xFF);
    }

    inline void clear()
    {
        std::fill(m_bytes.get(), m_bytes.get() + m_byte_cnt, 0);
    }

    inline void activate_bit(const int bit_index)
    {
        assert(bit_index >= 0 && bit_index < m_byte_cnt * 8);
        m_bytes[bit_index / 8] |= static_cast<unsigned char>(1) << (bit_index % 8);
    }

    inline void deactivate_bit(const int bit_index)
    {
        assert(bit_index >= 0 && bit_index < m_byte_cnt * 8);
        m_bytes[bit_index / 8] &= ~(static_cast<unsigned char>(1) << (bit_index % 8));
    }

    inline bool is_bit_active(const int bit_index) const
    {
        assert(bit_index >= 0 && bit_index < m_byte_cnt * 8);
        return m_bytes[bit_index / 8] & (static_cast<unsigned char>(1) << (bit_index % 8));
    }

private:
    std::unique_ptr<cc::u_byte[]> m_bytes;
    int m_byte_cnt;
};

inline int incr_wrapped(const int val, const int incr, const int max_excl)
{
    const int max_wrapped = (val + incr) % max_excl;
    const int min_wrapped = max_wrapped < 0 ? max_wrapped + max_excl : max_wrapped;
    return min_wrapped;
}
