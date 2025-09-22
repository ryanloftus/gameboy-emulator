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

    /* References */
    memory *mem;
    uint8_t *code;
} virtual_cpu;

void create_virtual_cpu(virtual_cpu *cpu, memory *mem, uint8_t *code);
void fetch_execute(virtual_cpu *cpu);

#endif
