#include "debug.h"

#include <stdlib.h>
#include <stdio.h>

/* Global debug mode flag — defaults to off; set to true by --debug flag in main */
bool g_debug_mode = false;

void bad_assert(const char *expr, const char *file, const char *func, int line)
{
    printf("ASSERTION FAILED %s at %s:%s:%d\n", expr, file, func, line);
    exit(1);
}
