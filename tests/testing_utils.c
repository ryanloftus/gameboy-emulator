#include "testing_utils.h"

#include <stdio.h>

void bad_assert(const char *expr, const char *file, const char *func, int line)
{
    printf("Assertion Failed: %s at %s:%s:%d\n", expr, file, func, line);
}
