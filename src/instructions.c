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
const uint8_t R8_ID_F = 8;

uint8_t* get_r8(virtual_cpu *cpu, uint8_t r8_id)
{
    switch (r8_id)
    {
        case R8_ID_B: return &(cpu->b);
        case R8_ID_C: return &(cpu->c);
        case R8_ID_D: return &(cpu->d);
        case R8_ID_E: return &(cpu->e);
        case R8_ID_H: return &(cpu->h);
        case R8_ID_L: return &(cpu->l);
        case R8_ID_A: return &(cpu->a);
        case R8_ID_F: return &(cpu->f);
        case R8_ID_MEM_HL: return access_memory8(cpu->mem, cpu->hl);
        default: debug_assert(0);
    }

    return NULL;
}

uint16_t* get_r16(virtual_cpu *cpu, uint8_t r16_id)
{
    switch (r16_id)
    {
        case 0: return &(cpu->bc);
        case 1: return &(cpu->de);
        case 2: return &(cpu->hl);
        case 3: return &(cpu->sp);
        default: debug_assert(0);
    }

    return NULL;
}

const uint8_t FLAG_CARRY = 4;
const uint8_t FLAG_HALF_CARRY = 5; // TODO
const uint8_t FLAG_SUBTRACTION = 6; // TODO: clear zero flag on non-subtraction block 0 instr
const uint8_t FLAG_ZERO = 7;

uint8_t get_flag(virtual_cpu *cpu, uint8_t flag)
{
    debug_assert(flag >= 4 && flag <= 7);
    return (cpu->f & (1 << flag)) >> flag;
}

void set_flag(virtual_cpu *cpu, uint8_t flag)
{
    debug_assert(flag >= 4 && flag <= 7);
    cpu->f |= (1 << flag);
}

void clear_flag(virtual_cpu *cpu, uint8_t flag)
{
    debug_assert(flag >= 4 && flag <= 7);
    cpu->f ^= (1 << flag);
}

void perform_8bit_add(virtual_cpu *cpu, uint8_t *dest, uint8_t addend1, uint8_t addend2)
{
    uint8_t cpu_sum = addend1 + addend2;

    if (cpu_sum == 0)
    {
        set_flag(cpu, FLAG_ZERO);
    }

    if (cpu_sum < addend1)
    {
        set_flag(cpu, FLAG_CARRY);
    }

    if (dest != NULL)
    {
        *dest = cpu_sum;
    }
}

void perform_8bit_sub(virtual_cpu *cpu, uint8_t *dest, uint8_t minuend, uint8_t subtrahend)
{
    uint8_t cpu_difference = minuend - subtrahend;

    if (cpu_difference == 0)
    {
        set_flag(cpu, FLAG_ZERO);
    }

    if (subtrahend > minuend)
    {
        set_flag(cpu, FLAG_CARRY);
    }

    set_flag(cpu, FLAG_SUBTRACTION);

    if (dest != NULL)
    {
        *dest = cpu_difference;
    }
}

void perform_16bit_add(virtual_cpu *cpu, uint16_t *dest, uint16_t addend1, uint16_t addend2)
{
    uint16_t cpu_sum = addend1 + addend2;

    if (cpu_sum == 0)
    {
        set_flag(cpu, FLAG_ZERO);
    }

    if (cpu_sum < addend1)
    {
        set_flag(cpu, FLAG_CARRY);
    }

    if (dest != NULL)
    {
        *dest = cpu_sum;
    }
}

void perform_16bit_sub(virtual_cpu *cpu, uint16_t *dest, uint16_t minuend, uint16_t subtrahend)
{
    uint16_t cpu_difference = minuend - subtrahend;

    if (cpu_difference == 0)
    {
        set_flag(cpu, FLAG_ZERO);
    }

    if (subtrahend > minuend)
    {
        set_flag(cpu, FLAG_CARRY);
    }

    set_flag(cpu, FLAG_SUBTRACTION);

    if (dest != NULL)
    {
        *dest = cpu_difference;
    }
}

void perform_8bit_and(virtual_cpu *cpu, uint8_t *dest, uint8_t operand1, uint8_t operand2)
{
    uint8_t result = operand1 & operand2;

    if (result == 0)
    {
        set_flag(cpu, FLAG_ZERO);
    }

    if (dest != NULL)
    {
        *dest = result;
    }
}

void perform_8bit_xor(virtual_cpu *cpu, uint8_t *dest, uint8_t operand1, uint8_t operand2)
{
    uint8_t result = operand1 ^ operand2;

    if (result == 0)
    {
        set_flag(cpu, FLAG_ZERO);
    }

    if (dest != NULL)
    {
        *dest = result;
    }
}

void perform_8bit_or(virtual_cpu *cpu, uint8_t *dest, uint8_t operand1, uint8_t operand2)
{
    uint8_t result = operand1 | operand2;

    if (result == 0)
    {
        set_flag(cpu, FLAG_ZERO);
    }

    if (dest != NULL)
    {
        *dest = result;
    }
}

/*
 * ----------------------------------------------------------------
 * Block Zero Instructions
 * ----------------------------------------------------------------
 */

void nop(virtual_cpu *cpu, uint8_t opcode)
{
    (void)cpu;
    (void)opcode;
}

void inc_r8(virtual_cpu *cpu, uint8_t opcode)
{
    uint8_t r8_id = (opcode & 0b111000) >> 3;
    uint8_t *r8 = get_r8(cpu, r8_id);
    perform_8bit_add(cpu, r8, *r8, 1);
}

void dec_r8(virtual_cpu *cpu, uint8_t opcode)
{
    uint8_t r8_id = (opcode & 0b111000) >> 3;
    uint8_t *r8 = get_r8(cpu, r8_id);
    perform_8bit_sub(cpu, r8, *r8, 1);
}

void inc_r16(virtual_cpu *cpu, uint8_t opcode)
{
    uint8_t r16_id = (opcode & 0b110000) >> 4;
    uint16_t *r16 = get_r16(cpu, r16_id);
    perform_16bit_add(cpu, r16, *r16, 1);
}

void dec_r16(virtual_cpu *cpu, uint8_t opcode)
{
    uint8_t r16_id = (opcode & 0b110000) >> 4;
    uint16_t *r16 = get_r16(cpu, r16_id);
    perform_16bit_sub(cpu, r16, *r16, 1);
}

void add_hl_r16(virtual_cpu *cpu, uint8_t opcode)
{
    uint8_t r16_id = (opcode & 0b110000) >> 4;
    uint16_t r16 = *get_r16(cpu, r16_id);
    perform_16bit_add(cpu, &(cpu->hl), cpu->hl, r16);
}

void ld_r16_imm16(virtual_cpu *cpu, uint8_t opcode)
{
    uint16_t *r16 = get_r16(cpu, (opcode >> 4) & 0b11);
    uint16_t imm16 = cpu->code[cpu->pc + 1] | (cpu->code[cpu->pc + 2] << 8);
    *r16 = imm16;
}

void ld_r8_imm8(virtual_cpu *cpu, uint8_t opcode)
{
    uint16_t *r8 = get_r8(cpu, (opcode >> 3) & 0b111);
    uint16_t imm8 = cpu->code[cpu->pc + 1];
    *r8 = imm8;
}

void ld_r16mem_a(virtual_cpu *cpu, uint8_t opcode)
{
    //TODO
    printf("not implemented\n");
}

void ld_a_r16mem(virtual_cpu *cpu, uint8_t opcode)
{
    //TODO
    printf("not implemented\n");
}

void ld_imm16_sp(virtual_cpu *cpu, uint8_t opcode)
{
    //TODO
    printf("not implemented\n");
}

void rlca(virtual_cpu *cpu, uint8_t opcode)
{
    uint8_t *a = get_r8(cpu, R8_ID_A);
    if ((*a) & (1 << 7))
    {
        set_flag(cpu, FLAG_CARRY);
    }
    else
    {
        clear_flag(cpu, FLAG_CARRY);
    }
    *a = ((*a) << 1) | get_flag(cpu, FLAG_CARRY);

    clear_flag(cpu, FLAG_ZERO);
    clear_flag(cpu, FLAG_HALF_CARRY);
    clear_flag(cpu, FLAG_SUBTRACTION);
}

void rrca(virtual_cpu *cpu, uint8_t opcode)
{
    uint8_t *a = get_r8(cpu, R8_ID_A);
    if ((*a) & 1)
    {
        set_flag(cpu, FLAG_CARRY);
    }
    else
    {
        clear_flag(cpu, FLAG_CARRY);
    }
    *a = ((*a) >> 1) | (get_flag(cpu, FLAG_CARRY) << 7);

    clear_flag(cpu, FLAG_ZERO);
    clear_flag(cpu, FLAG_HALF_CARRY);
    clear_flag(cpu, FLAG_SUBTRACTION);
}

void rla(virtual_cpu *cpu, uint8_t opcode)
{
    uint8_t *a = get_r8(cpu, R8_ID_A);
    uint8_t a_val = *a;
    *a = (a_val << 1) | get_flag(cpu, FLAG_CARRY);
    if (a_val & (1 << 7))
    {
        set_flag(cpu, FLAG_CARRY);
    }
    else
    {
        clear_flag(cpu, FLAG_CARRY);
    }

    clear_flag(cpu, FLAG_ZERO);
    clear_flag(cpu, FLAG_HALF_CARRY);
    clear_flag(cpu, FLAG_SUBTRACTION);
}

void rra(virtual_cpu *cpu, uint8_t opcode)
{
    uint8_t *a = get_r8(cpu, R8_ID_A);
    uint8_t a_val = *a;
    *a = (a_val >> 1) | (get_flag(cpu, FLAG_CARRY) << 7);
    if (a_val & 1)
    {
        set_flag(cpu, FLAG_CARRY);
    }
    else
    {
        clear_flag(cpu, FLAG_CARRY);
    }

    clear_flag(cpu, FLAG_ZERO);
    clear_flag(cpu, FLAG_HALF_CARRY);
    clear_flag(cpu, FLAG_SUBTRACTION);
}

void daa(virtual_cpu *cpu, uint8_t opcode)
{
    //TODO
    printf("not implemented\n");
}

void cpl(virtual_cpu *cpu, uint8_t opcode)
{
    //TODO
    printf("not implemented\n");
}

void scf(virtual_cpu *cpu, uint8_t opcode)
{
    //TODO
    printf("not implemented\n");
}

void ccf(virtual_cpu *cpu, uint8_t opcode)
{
    //TODO
    printf("not implemented\n");
}

void jr_imm8(virtual_cpu *cpu, uint8_t opcode)
{
    //TODO
    printf("not implemented\n");
}

void jr_cond_imm8(virtual_cpu *cpu, uint8_t opcode)
{
    //TODO
    printf("not implemented\n");
}

void stop(virtual_cpu *cpu, uint8_t opcode)
{
    //TODO
    printf("not implemented\n");
}

const instruction block_zero_instructions[] =
{
    {
        0b111111,
        0b000000,
        1,
        1,
        nop
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
    },
    {
        0b001111,
        0b000001,
        3,
        3,
        ld_r16_imm16
    },
    {
        0b000111,
        0b000110,
        2,
        2,
        ld_r8_imm8
    },
    {
        0b111111,
        0b000111,
        1,
        1,
        rlca
    },
    {
        0b111111,
        0b001111,
        1,
        1,
        rrca
    },
    {
        0b111111,
        0b010111,
        1,
        1,
        rla
    },
    {
        0b111111,
        0b011111,
        1,
        1,
        rra
    }
};

const size_t block_zero_instructions_count = sizeof(block_zero_instructions) / sizeof(instruction);

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
    // subtraction flag is only used by daa instruction in block zero, so we can clear it here
    clear_flag(cpu, FLAG_SUBTRACTION);

    cpu->pc += 1;

    if (opcode == 0b01110110)
    {
        printf("halt instruction not implemented\n");
        return;
    }

    uint8_t src_r8_id = opcode & 0b111;
    uint8_t dest_r8_id = (opcode >> 3) & 0b111;

    uint8_t src = *get_r8(cpu, src_r8_id);
    uint8_t *dest = get_r8(cpu, dest_r8_id);

    *dest = src;
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
    // subtraction flag is only used by daa instruction in block zero, so we can clear it here
    clear_flag(cpu, FLAG_SUBTRACTION);

    uint8_t r8_id = opcode & 0b111;
    uint8_t r8 = *get_r8(cpu, r8_id);
    uint8_t *a = &(cpu->a);

    uint8_t three_bit_opcode = (opcode >> 3) & 0b111;

    switch (three_bit_opcode)
    {
        case BLOCK_TWO_3BIT_OPCODE_ADD_A_R8:
            perform_8bit_add(cpu, a, *a, r8);
            break;
        case BLOCK_TWO_3BIT_OPCODE_ADC_A_R8:
            perform_8bit_add(cpu, a, *a, r8 + get_flag(cpu, FLAG_CARRY));
            break;
        case BLOCK_TWO_3BIT_OPCODE_SUB_A_R8:
            perform_8bit_sub(cpu, a, *a, r8);
            break;
        case BLOCK_TWO_3BIT_OPCODE_SBC_A_R8:
            perform_8bit_sub(cpu, a, *a, r8 + get_flag(cpu, FLAG_CARRY));
            break;
        case BLOCK_TWO_3BIT_OPCODE_AND_A_R8:
            perform_8bit_and(cpu, a, *a, r8);
            break;
        case BLOCK_TWO_3BIT_OPCODE_XOR_A_R8:
            perform_8bit_xor(cpu, a, *a, r8);
            break;
        case BLOCK_TWO_3BIT_OPCODE_OR_A_R8:
            perform_8bit_or(cpu, a, *a, r8);
            break;
        case BLOCK_TWO_3BIT_OPCODE_CP_A_R8:
            perform_8bit_sub(cpu, NULL, *a, r8);
            break;
        default:
            printf("invalid block two instruction");
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
    // subtraction flag is only used by daa instruction in block zero, so we can clear it here
    clear_flag(cpu, FLAG_SUBTRACTION);
    (void)opcode;
    printf("block three instructions not implemented\n");
}
