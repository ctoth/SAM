// Debug Logging

#include "debug.h"
#include <varargs.h>

int DebugPrint(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int ret = vprintf(STDERR format, args);
    va_end(args);
    return ret;
}