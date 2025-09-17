#include "cpu.h"
#include "debug.h"

#include <stdio.h>
#include <memory.h>

uint8_t* get_r8(virtual_cpu *cpu, uint8_t r8_id)
{
    switch (r8_id)
    {
        case 0:
            return (uint8_t*)&(cpu->bc);
        case 1:
            return (uint8_t*)&(cpu->bc) + 1;
        case 2:
            return (uint8_t*)&(cpu->de);
        case 3:
            return (uint8_t*)&(cpu->de) + 1;
        case 4:
            return (uint8_t*)&(cpu->hl);
        case 5:
            return (uint8_t*)&(cpu->hl) + 1;
        case 6:
            // TODO: update this when memory is implemented
            debug_assert(0);
        case 7:
            return (uint8_t*)&(cpu->af);
        default:
            debug_assert(0);
    }

    return NULL;
}

void noop()
{
    return;
}

void inc_r8(virtual_cpu *cpu, uint8_t r8_id)
{
    uint8_t *r8 = get_r8(cpu, r8_id);
    (*r8)++;
    return;
}

void dec_r8(virtual_cpu *cpu, uint8_t r8_id)
{
    uint8_t *r8 = get_r8(cpu, r8_id);
    (*r8)--;
    return;
}

void execute_block_zero_instruction(virtual_cpu *cpu, uint8_t opcode)
{
    if (opcode == 0)
    {
        noop();
        cpu->pc += 1;
        return;
    }
    else if ((opcode & 0b111) == 4)
    {
        uint8_t r8 = (opcode & 0b111000) >> 3;
        inc_r8(cpu, r8);
        cpu->pc += 1;
        return;
    }
    else if ((opcode & 0b111) == 5)
    {
        uint8_t r8 = (opcode & 0b111000) >> 3;
        dec_r8(cpu, r8);
        cpu->pc += 1;
        return;
    }
    else
    {
        printf("unimplemented opcode %d\n", opcode);
        return;
    }
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
