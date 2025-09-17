#ifndef _DEBUG_H_
#define _DEBUG_H_

void bad_assert(const char *expr, const char *file, const char *func, int line);

#define debug_assert(expr) ((expr) ? (void)0 : bad_assert(#expr, __FILE__, __func__, __LINE__))

#endif
