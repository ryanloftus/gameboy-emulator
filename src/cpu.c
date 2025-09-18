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

uint16_t* get_r16(virtual_cpu *cpu, uint8_t r16_id)
{
    switch (r16_id)
    {
        case 0:
            return &(cpu->bc);
        case 1:
            return &(cpu->de);
        case 2:
            return &(cpu->hl);
        case 3:
            return &(cpu->sp);
        case 4:
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

void inc_r16(virtual_cpu *cpu, uint8_t r16_id)
{
    uint16_t *r16 = get_r16(cpu, r16_id);
    (*r16)++;
    return;
}

void dec_r16(virtual_cpu *cpu, uint8_t r16_id)
{
    uint16_t *r16 = get_r16(cpu, r16_id);
    (*r16)--;
    return;
}

void execute_block_zero_instruction(virtual_cpu *cpu, uint8_t opcode)
{
    if (opcode == 0)
    {
        noop();
        cpu->pc += 1;
    }
    else if ((opcode & 0b111) == 4)
    {
        uint8_t r8 = (opcode & 0b111000) >> 3;
        inc_r8(cpu, r8);
        cpu->pc += 1;
    }
    else if ((opcode & 0b111) == 5)
    {
        uint8_t r8 = (opcode & 0b111000) >> 3;
        dec_r8(cpu, r8);
        cpu->pc += 1;
    }
    else if ((opcode & 0b1111) == 0b11)
    {
        uint8_t r16 = (opcode & 0b110000) >> 4;
        inc_r16(cpu, r16);
        cpu->pc += 1;
    }
    else if ((opcode & 0b1111) == 0b1011)
    {
        uint8_t r16 = (opcode & 0b110000) >> 4;
        dec_r16(cpu, r16);
        cpu->pc += 1;
    }
    else
    {
        printf("unimplemented opcode %d\n", opcode);
    }

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
