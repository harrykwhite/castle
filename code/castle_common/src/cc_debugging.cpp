#include <castle_common/cc_debugging.h>

#include <cstdio>
#include <cstdarg>

namespace cc
{

void log(const char *const msg, ...)
{
    va_list args;
    va_start(args, msg);

    vprintf(msg, args);
    printf("\n");

    va_end(args);
}

void log_error(const char *const msg, ...)
{
    va_list args;
    va_start(args, msg);

    fprintf(stderr, "ERROR: ");
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");

    va_end(args);
}

}
