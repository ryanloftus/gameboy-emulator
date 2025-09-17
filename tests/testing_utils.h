#ifndef _TESTING_UTILS_H_
#define _TESTING_UTILS_H_

#define PASSED 0
#define FAILED 1

void run_tests(int (*tests[])(void), int num_tests);

void test_assert(const char *expr, const char *file, const char *func, int line);

#define assert(expr) \
    do { \
        if (!(expr)) { \
            test_assert(#expr, __FILE__, __func__, __LINE__); \
            return FAILED; \
        } \
    } while (0)

#endif
