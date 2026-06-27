#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdbool.h>
#include <stdint.h>

#include "decode.h"
#include "mmu.h"

#define DEBUG_LOG_PATH "gameboy.trace.log"

void bad_assert(const char *expr, const char *file, const char *func, int line);

void pretty_print_instr(const decoded_instr *instr, memory *mem, uint16_t pc);

void init_debug_log(void);
void close_debug_log(void);
void debug_log_printf(const char *fmt, ...);
void instr_log_printf(const char *fmt, ...);
void debug_log_instruction(uint16_t pc, const decoded_instr *instr, memory *mem);
void serial_transmit_byte(uint8_t byte);

#define debug_assert(expr) ((expr) ? (void)0 : bad_assert(#expr, __FILE__, __func__, __LINE__))

/* Global debug mode flag — set in main, disables SDL and enables test-ROM serial handling */
extern bool g_debug_mode;

/* When set, instruction disassembly is written to DEBUG_LOG_PATH */
extern bool g_instr_log;

#endif
