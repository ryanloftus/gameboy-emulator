#ifndef _CPU_H_
#define _CPU_H_

#include "mmu.h"

#include <stdint.h>

typedef struct virtual_cpu
{
    /* Registers */
    uint16_t pc; // Program Counter
    uint16_t sp; // Stack Pointer
    union
    {
        // Accumulator and Flags
        struct { uint8_t f; uint8_t a; };
        uint16_t af;
    };
    union
    {
        struct { uint8_t c; uint8_t b; };
        uint16_t bc;
    };
    union
    {
        struct { uint8_t e; uint8_t d; };
        uint16_t de;
    };
    union
    {
        struct { uint8_t l; uint8_t h; };
        uint16_t hl;
    };

    /* CPU timing */
    uint64_t cycles;

    /* EI one-instruction delay scheduling flag */
    uint8_t ei_scheduled; /* non-zero when EI has been executed but not yet taken effect */
    /* Interrupt Master Enable (IME) is separate from the IE register at
       memory address 0xFFFF. */
    uint8_t ime;

    /* CPU state flags */
    uint8_t is_halted;
    uint8_t is_stopped;

    /* References */
    memory *mem;
} virtual_cpu;

void create_virtual_cpu(virtual_cpu *cpu, memory *mem);
void fetch_execute(virtual_cpu *cpu);
void update_timers(virtual_cpu *cpu, uint16_t cycles_elapsed);
void tima_on_tac_write(memory *mem, uint8_t old_tac, uint8_t new_tac);
void tima_on_div_write(memory *mem);

#endif
