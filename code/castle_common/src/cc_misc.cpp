#include <castle_common/cc_misc.h>

namespace cc
{

int copy_str_and_get_len(const char *const src, char *const dest, const int max_len)
{
    for (int i = 0; i <= max_len; i++)
    {
        if (!(dest[i] = src[i]))
        {
            return i;
        }
    }

    return -1;
}

}
