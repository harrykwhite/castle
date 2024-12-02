#pragma once

#include <string.h>
#include <stdlib.h>
#include <assert.h>

namespace cc
{

using Byte = unsigned char;

struct MemArena
{
    Byte *buf;
    int size;
    int offs;
};

bool init_mem_arena(MemArena &arena, const int size);

inline void clean_mem_arena(MemArena &arena)
{
    assert(arena.buf);
    free(arena.buf);
    arena = {};
}

template<typename T>
T *push_to_mem_arena(MemArena &arena, const int cnt = 1)
{
    assert(arena.buf);

    const int pushSize = sizeof(T) * cnt;

    if (arena.offs + pushSize > arena.size)
    {
        assert(false);
        return nullptr;
    }

    if (cnt > 0)
    {
        T *const ptr = reinterpret_cast<T *>(arena.buf + arena.offs);
        arena.offs += pushSize;
        return ptr;
    }

    assert(!cnt);

    return nullptr;
}

inline void clear_mem_arena(MemArena &arena)
{
    assert(arena.buf);
    //memset(arena.buf, 0, arena.size);
    arena.offs = 0;
}

}
