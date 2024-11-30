#include <castle_common/cc_mem.h>

namespace cc
{

bool init_mem_arena(MemArena &arena, const int size)
{
    assert(size > 0);

    arena.buf = static_cast<Byte *>(malloc(size));

    if (!arena.buf)
    {
        return false;
    }

    memset(arena.buf, 0, size);

    arena.size = size;

    return true;
}

}
