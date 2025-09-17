#ifndef _CPU_H_
#define _CPU_H_

#include <stdint.h>

typedef struct virtual_cpu
{
    /* Registers */
    uint16_t pc; // Program Counter
    uint16_t sp; // Stack Pointer
    uint16_t af; // Accumulator and Flags
    uint16_t bc;
    uint16_t de;
    uint16_t hl;
} virtual_cpu;

void create_virtual_cpu(virtual_cpu *cpu);
void fetch_execute(virtual_cpu *cpu, uint8_t *code);

#endif
