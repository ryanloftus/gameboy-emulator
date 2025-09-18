#include "instructions.h"
#include "debug.h"

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

void noop(virtual_cpu *cpu, uint8_t opcode)
{
    (void)cpu;
    (void)opcode;
}

void inc_r8(virtual_cpu *cpu, uint8_t opcode)
{
    uint8_t r8_id = (opcode & 0b111000) >> 3;
    uint8_t *r8 = get_r8(cpu, r8_id);
    (*r8)++;
}

void dec_r8(virtual_cpu *cpu, uint8_t opcode)
{
    uint8_t r8_id = (opcode & 0b111000) >> 3;
    uint8_t *r8 = get_r8(cpu, r8_id);
    (*r8)--;
}

void inc_r16(virtual_cpu *cpu, uint8_t opcode)
{
    uint8_t r16_id = (opcode & 0b110000) >> 4;
    uint16_t *r16 = get_r16(cpu, r16_id);
    (*r16)++;
}

void dec_r16(virtual_cpu *cpu, uint8_t opcode)
{
    uint8_t r16_id = (opcode & 0b110000) >> 4;
    uint16_t *r16 = get_r16(cpu, r16_id);
    (*r16)--;
}

void add_hl_r16(virtual_cpu *cpu, uint8_t opcode)
{
    uint8_t r16_id = (opcode & 0b110000) >> 4;
    uint16_t r16 = *get_r16(cpu, r16_id);
    cpu->hl += r16;
}

const Instruction block_zero_instructions[] =
{
    {
        0b111111,
        0b000000,
        1,
        1,
        noop
    },
    {
        0b000111,
        0b000100,
        1,
        1,
        inc_r8
    },
    {
        0b000111,
        0b000101,
        1,
        1,
        dec_r8
    },
    {
        0b001111,
        0b000011,
        1,
        1,
        inc_r16
    },
    {
        0b001111,
        0b001011,
        1,
        1,
        dec_r16
    },
    {
        0b001111,
        0b001001,
        1,
        1,
        add_hl_r16
    }
};

const size_t block_zero_instructions_count = sizeof(block_zero_instructions) / sizeof(Instruction);
