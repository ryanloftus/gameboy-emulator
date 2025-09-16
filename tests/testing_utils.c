#include "testing_utils.h"

#include <stdio.h>

void run_tests(int (*tests[])(void), int num_tests)
{
    // TODO print the name of the test suite
    printf("\n\nRunning tests\n\n");

    int tests_passed = 0;
    int tests_failed = 0;

    for (int i = 0; i < num_tests; ++i)
    {
        if (tests[i]() == PASSED)
        {
            tests_passed++;
        }
        else
        {
            tests_failed++;
        }
    }

    printf("Passed: %d / %d tests\n", tests_passed, num_tests);

    if (tests_failed)
    {
        printf("\033[31mFailed: %d tests\033[37m\n", tests_failed);
    }

    return;
}

void bad_assert(const char *expr, const char *file, const char *func, int line)
{
    printf("Assertion Failed: %s at %s:%s:%d\n", expr, file, func, line);
}
