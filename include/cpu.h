#ifndef _CPU_H_
#define _CPU_H_

#include <stdint.h>

typedef struct virtual_cpu
{
    /* Registers */
    uint16_t pc; // Program Counter
    uint16_t sp; // Stack Pointer
    uint8_t a; // Accumulator
    uint8_t f; // Flags
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t h;
    uint8_t l;
} virtual_cpu;

void create_virtual_cpu(virtual_cpu *cpu);
void fetch_execute(virtual_cpu *cpu, uint8_t *code);

#endif
