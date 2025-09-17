#include "debug.h"

#include <stdlib.h>
#include <stdio.h>

void bad_assert(const char *expr, const char *file, const char *func, int line)
{
    printf("ASSERTION FAILED %s at %s:%s:%d\n", expr, file, func, line);
    exit(1);
}
