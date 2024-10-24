#pragma once

#include <glad/glad.h>

namespace ce
{

using u_gl_id = GLuint;

#if 0
struct s_mem_arena
{
    void *buf;
    int buf_size;
    int buf_offs;
    int last_alloc_size; // (Stored for rewinding functionality.)
};

template<typename T>
inline T *get_buf_array_at_offs(const void *const buf, const int offs)
{
    return reinterpret_cast<T *>(reinterpret_cast<char *>(buf) + offs);
}

#endif

template<int BIT_CNT>
struct s_bitset
{
    unsigned char bytes[BIT_CNT / 8];

    inline bool is_bit_active(const int bit_index) const
    {
        return bytes[bit_index / 8] & (static_cast<unsigned char>(1) << (bit_index % 8));
    }

    inline void activate_bit(const int bit_index)
    {
        bytes[bit_index / 8] |= static_cast<unsigned char>(1) << (bit_index % 8);
    }

    inline void deactivate_bit(const int bit_index)
    {
        bytes[bit_index / 8] &= ~(static_cast<unsigned char>(1) << (bit_index % 8));
    }
};

#if 0
bool init_mem_arena(s_mem_arena *const mem_arena, const int buf_size);
void *mem_arena_alloc(s_mem_arena *const mem_arena, const int buf_size);
void reset_mem_arena(s_mem_arena *const mem_arena);
void rewind_mem_arena(s_mem_arena *const mem_arena);
void clean_mem_arena(s_mem_arena *const mem_arena);

#endif
}
