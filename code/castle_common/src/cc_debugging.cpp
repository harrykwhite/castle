#include <castle_common/cc_debugging.h>

#include <stdio.h>
#include <stdarg.h>

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

void log_warning(const char *const msg, ...)
{
    va_list args;
    va_start(args, msg);

    fprintf(stderr, "WARNING: ");
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");

    va_end(args);
}

}
