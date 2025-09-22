#ifndef _INSTRUCTIONS_H_
#define _INSTRUCTIONS_H_

#include <stdint.h>
#include <stddef.h>

#include "cpu.h"
#include "mmu.h"

typedef struct
{
    uint8_t bitmask;
    uint8_t pattern;
    uint8_t cycles;
    uint8_t bytes;
    void (*exec)(virtual_cpu *, memory *, uint8_t);
} instruction;

extern const instruction block_zero_instructions[];
extern const size_t block_zero_instructions_count;

void execute_block_zero_instruction(virtual_cpu *cpu, memory *mem, uint8_t opcode);
void execute_block_one_instruction(virtual_cpu *cpu, memory *mem, uint8_t opcode);
void execute_block_two_instruction(virtual_cpu *cpu, memory *mem, uint8_t opcode);
void execute_block_three_instruction(virtual_cpu *cpu, memory *mem, uint8_t opcode);


#endif
