#include "cpu.h"
#include "instructions.h"
#include "debug.h"

#include <stdio.h>
#include <memory.h>

void create_virtual_cpu(virtual_cpu *cpu)
{
    memset(cpu, 0, sizeof(virtual_cpu));
}

void fetch_execute(virtual_cpu *cpu, memory *mem, uint8_t *code)
{
    uint8_t opcode = code[cpu->pc];
    uint8_t block = (opcode & 0b11000000) >> 6;

    switch (block)
    {
        case 0:
            execute_block_zero_instruction(cpu, mem, opcode);
            break;
        case 1:
            execute_block_one_instruction(cpu, mem, opcode);
            break;
        case 2:
            execute_block_two_instruction(cpu, mem, opcode);
            break;
        case 3:
            execute_block_three_instruction(cpu, mem, opcode);
            break;
        default:
            printf("unimplemented opcode %d\n", opcode);
            break;
    }

    return;
}
