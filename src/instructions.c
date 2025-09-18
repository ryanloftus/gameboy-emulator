#include "instructions.h"
#include "debug.h"

#include <stdio.h>

const uint8_t R8_ID_B = 0;
const uint8_t R8_ID_C = 1;
const uint8_t R8_ID_D = 2;
const uint8_t R8_ID_E = 3;
const uint8_t R8_ID_H = 4;
const uint8_t R8_ID_L = 5;
const uint8_t R8_ID_MEM_HL = 6;
const uint8_t R8_ID_A = 7;

uint8_t* get_r8(virtual_cpu *cpu, uint8_t r8_id)
{
    switch (r8_id)
    {
        case R8_ID_B:
            return (uint8_t*)&(cpu->bc);
        case R8_ID_C:
            return (uint8_t*)&(cpu->bc) + 1;
        case R8_ID_D:
            return (uint8_t*)&(cpu->de);
        case R8_ID_E:
            return (uint8_t*)&(cpu->de) + 1;
        case R8_ID_H:
            return (uint8_t*)&(cpu->hl);
        case R8_ID_L:
            return (uint8_t*)&(cpu->hl) + 1;
        case R8_ID_MEM_HL:
            // TODO: update this when memory is implemented
            debug_assert(0);
        case R8_ID_A:
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

/*
 * ----------------------------------------------------------------
 * Block Zero Instructions
 * ----------------------------------------------------------------
 */

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

/*
 * ----------------------------------------------------------------
 * Block One Instructions
 * ----------------------------------------------------------------
 */

void execute_block_one_instruction(virtual_cpu *cpu, uint8_t opcode)
{
    (void)cpu;
    (void)opcode;
    printf("block one instructions not implemented\n");
}

/*
 * ----------------------------------------------------------------
 * Block Two Instructions
 * ----------------------------------------------------------------
 */

const uint8_t BLOCK_TWO_3BIT_OPCODE_ADD_A_R8 = 0;
const uint8_t BLOCK_TWO_3BIT_OPCODE_ADC_A_R8 = 1;
const uint8_t BLOCK_TWO_3BIT_OPCODE_SUB_A_R8 = 2;
const uint8_t BLOCK_TWO_3BIT_OPCODE_SBC_A_R8 = 3;
const uint8_t BLOCK_TWO_3BIT_OPCODE_AND_A_R8 = 4;
const uint8_t BLOCK_TWO_3BIT_OPCODE_XOR_A_R8 = 5;
const uint8_t BLOCK_TWO_3BIT_OPCODE_OR_A_R8 = 6;
const uint8_t BLOCK_TWO_3BIT_OPCODE_CP_A_R8 = 7;

void execute_block_two_instruction(virtual_cpu *cpu, uint8_t opcode)
{
    uint8_t r8_id = opcode & 0b111;
    uint8_t r8 = *get_r8(cpu, r8_id);
    uint8_t *a = get_r8(cpu, R8_ID_A);

    uint8_t three_bit_opcode = (opcode >> 3) & 0b111;

    switch (three_bit_opcode)
    {
        case BLOCK_TWO_3BIT_OPCODE_ADD_A_R8:
            *a += r8;
            break;
        case BLOCK_TWO_3BIT_OPCODE_ADC_A_R8:
            // TODO: implement carry bit functionality
            printf("unimplemented adc a r8");
            break;
        case BLOCK_TWO_3BIT_OPCODE_SUB_A_R8:
            *a -= r8;
            break;
        default:
            printf("unimplemented block two instruction");
            break;
    }

    cpu->pc += 1;
}

/*
 * ----------------------------------------------------------------
 * Block Three Instructions
 * ----------------------------------------------------------------
 */

void execute_block_three_instruction(virtual_cpu *cpu, uint8_t opcode)
{
    (void)cpu;
    (void)opcode;
    printf("block three instructions not implemented\n");
}
