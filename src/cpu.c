#include "cpu.h"
#include "instructions.h"
#include "debug.h"

#include <stdio.h>
#include <memory.h>

void execute_block_zero_instruction(virtual_cpu *cpu, uint8_t opcode)
{
    for (size_t i = 0; i < block_zero_instructions_count; ++i)
    {
        if ((opcode & block_zero_instructions[i].bitmask) == block_zero_instructions[i].pattern)
        {
            block_zero_instructions[i].exec(cpu, opcode);
            cpu->pc += block_zero_instructions[i].bytes;
            return;
        }
    }

    printf("unimplemented opcode %d\n", opcode);
    return;
}

void create_virtual_cpu(virtual_cpu *cpu)
{
    memset(cpu, 0, sizeof(virtual_cpu));
}

void fetch_execute(virtual_cpu *cpu, uint8_t *code)
{
    uint8_t opcode = code[cpu->pc];
    uint8_t block = (opcode & 0b11000000) >> 6;

    switch (block)
    {
        case 0:
            execute_block_zero_instruction(cpu, opcode);
            break;
        case 1:
        case 2:
        case 3:
        default:
            printf("unimplemented opcode %d\n", opcode);
            break;
    }

    return;
}
