#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdbool.h>
#include <stdint.h>

#include "decode.h"
#include "mmu.h"

void bad_assert(const char *expr, const char *file, const char *func, int line);

void pretty_print_instr(const decoded_instr *instr, memory *mem, uint16_t pc);

#define debug_assert(expr) ((expr) ? (void)0 : bad_assert(#expr, __FILE__, __func__, __LINE__))

/* Global debug mode flag — set in main, checked in cpu.c to enable verbose logging */
extern bool g_debug_mode;

#endif
