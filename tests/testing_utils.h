#ifndef _TESTING_UTILS_H_
#define _TESTING_UTILS_H_


void bad_assert(const char *expr, const char *file, const char *func, int line);

#define assert(expr) \
    do { \
        bad_assert(#expr, __FILE__, __func__, __LINE__); \
        if (!(expr)) return FAILED; \
    } while (0)

#endif
