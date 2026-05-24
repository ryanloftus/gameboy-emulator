#include "instructions.h"
#include "debug.h"

#include <stdio.h>

static const uint8_t R8_ID_B = 0;
static const uint8_t R8_ID_C = 1;
static const uint8_t R8_ID_D = 2;
static const uint8_t R8_ID_E = 3;
static const uint8_t R8_ID_H = 4;
static const uint8_t R8_ID_L = 5;
static const uint8_t R8_ID_MEM_HL = 6;
static const uint8_t R8_ID_A = 7;

static const uint8_t F_MASK_Z = 0x80;
static const uint8_t F_MASK_N = 0x40;
static const uint8_t F_MASK_H = 0x20;
static const uint8_t F_MASK_C = 0x10;
static const uint8_t F_LOWER_NIBBLE = 0x0F;

static uint8_t *get_r8(virtual_cpu *cpu, uint8_t r8_id)
{
    switch (r8_id)
    {
        case R8_ID_B: return &cpu->b;
        case R8_ID_C: return &cpu->c;
        case R8_ID_D: return &cpu->d;
        case R8_ID_E: return &cpu->e;
        case R8_ID_H: return &cpu->h;
        case R8_ID_L: return &cpu->l;
        case R8_ID_A: return &cpu->a;
        case R8_ID_MEM_HL: return access_memory8(cpu->mem, cpu->hl);
        default: debug_assert(0);
    }

    return NULL;
}

static uint16_t *get_r16(virtual_cpu *cpu, uint8_t r16_id)
{
    switch (r16_id)
    {
        case 0: return &cpu->bc;
        case 1: return &cpu->de;
        case 2: return &cpu->hl;
        case 3: return &cpu->sp;
        default: debug_assert(0);
    }

    return NULL;
}

/* r16mem: 0=BC, 1=DE, 2=HL+, 3=HL-. Sets *post_hl_step to -1, 0, or +1. */
static uint16_t r16mem_resolve(virtual_cpu *cpu, uint8_t r16mem_id, int8_t *post_hl_step)
{
    switch (r16mem_id)
    {
        case 0:
            *post_hl_step = 0;
            return cpu->bc;
        case 1:
            *post_hl_step = 0;
            return cpu->de;
        case 2:
            *post_hl_step = 1;
            return cpu->hl;
        case 3:
            *post_hl_step = -1;
            return cpu->hl;
        default:
            debug_assert(0);
            *post_hl_step = 0;
            return 0;
    }
}

static void r16mem_apply_hl_step(virtual_cpu *cpu, int8_t post_hl_step)
{
    if (post_hl_step > 0)
    {
        cpu->hl++;
    }
    else if (post_hl_step < 0)
    {
        cpu->hl--;
    }
}

static uint8_t flag_get(virtual_cpu *cpu, uint8_t mask)
{
    return (cpu->f & mask) != 0;
}

static void flags_write(virtual_cpu *cpu, uint8_t z, uint8_t n, uint8_t h, uint8_t c)
{
    cpu->f = (cpu->f & F_LOWER_NIBBLE)
        | (z ? F_MASK_Z : 0)
        | (n ? F_MASK_N : 0)
        | (h ? F_MASK_H : 0)
        | (c ? F_MASK_C : 0);
}

static void flags_write_preserve_c(virtual_cpu *cpu, uint8_t z, uint8_t n, uint8_t h)
{
    uint8_t c = flag_get(cpu, F_MASK_C);
    flags_write(cpu, z, n, h, c);
}

static void alu_inc_r8(virtual_cpu *cpu, uint8_t *dest)
{
    uint8_t value = *dest;
    uint8_t result = value + 1;
    *dest = result;
    flags_write_preserve_c(cpu, result == 0, 0, (value & 0x0F) == 0x0F);
}

static void alu_dec_r8(virtual_cpu *cpu, uint8_t *dest)
{
    uint8_t value = *dest;
    uint8_t result = value - 1;
    *dest = result;
    flags_write_preserve_c(cpu, result == 0, 1, (value & 0x0F) == 0);
}

static void alu_add_a_r8(virtual_cpu *cpu, uint8_t operand)
{
    uint8_t a = cpu->a;
    uint8_t result = a + operand;
    cpu->a = result;
    flags_write(cpu,
        result == 0,
        0,
        ((a & 0x0F) + (operand & 0x0F)) > 0x0F,
        result < a);
}

static void alu_adc_a_r8(virtual_cpu *cpu, uint8_t operand)
{
    uint8_t carry = flag_get(cpu, F_MASK_C);
    uint8_t a = cpu->a;
    uint16_t result = (uint16_t)a + operand + carry;
    cpu->a = (uint8_t)result;
    flags_write(cpu,
        (uint8_t)result == 0,
        0,
        ((a & 0x0F) + (operand & 0x0F) + carry) > 0x0F,
        result > 0xFF);
}

static void alu_sub_a_r8(virtual_cpu *cpu, uint8_t operand, uint8_t write_result)
{
    uint8_t a = cpu->a;
    uint8_t result = a - operand;
    if (write_result)
    {
        cpu->a = result;
    }
    flags_write(cpu,
        result == 0,
        1,
        (a & 0x0F) < (operand & 0x0F),
        a < operand);
}

static void alu_sbc_a_r8(virtual_cpu *cpu, uint8_t operand)
{
    uint8_t carry = flag_get(cpu, F_MASK_C);
    uint8_t a = cpu->a;
    uint16_t subtrahend = operand + carry;
    uint8_t result = a - (uint8_t)subtrahend;
    cpu->a = result;
    flags_write(cpu,
        result == 0,
        1,
        (a & 0x0F) < ((uint8_t)subtrahend & 0x0F),
        (uint16_t)a < subtrahend);
}

static void alu_and_a_r8(virtual_cpu *cpu, uint8_t operand)
{
    uint8_t result = cpu->a & operand;
    cpu->a = result;
    flags_write(cpu, result == 0, 0, 1, 0);
}

static void alu_xor_a_r8(virtual_cpu *cpu, uint8_t operand)
{
    uint8_t result = cpu->a ^ operand;
    cpu->a = result;
    flags_write(cpu, result == 0, 0, 0, 0);
}

static void alu_or_a_r8(virtual_cpu *cpu, uint8_t operand)
{
    uint8_t result = cpu->a | operand;
    cpu->a = result;
    flags_write(cpu, result == 0, 0, 0, 0);
}

static void alu_add_hl_r16(virtual_cpu *cpu, uint16_t operand)
{
    uint16_t hl = cpu->hl;
    uint16_t result = hl + operand;
    cpu->hl = result;
    uint8_t z = flag_get(cpu, F_MASK_Z);
    flags_write(cpu, z, 0, (hl & 0x0FFF) + (operand & 0x0FFF) > 0x0FFF, result < hl);
}

static void exec_nop(virtual_cpu *cpu)
{
    (void)cpu;
}

static void exec_inc_r8(virtual_cpu *cpu, const instr_operands *ops)
{
    alu_inc_r8(cpu, get_r8(cpu, ops->r8));
}

static void exec_dec_r8(virtual_cpu *cpu, const instr_operands *ops)
{
    alu_dec_r8(cpu, get_r8(cpu, ops->r8));
}

static void exec_inc_r16(virtual_cpu *cpu, const instr_operands *ops)
{
    uint16_t *r16 = get_r16(cpu, ops->r16);
    (*r16)++;
}

static void exec_dec_r16(virtual_cpu *cpu, const instr_operands *ops)
{
    uint16_t *r16 = get_r16(cpu, ops->r16);
    (*r16)--;
}

static void exec_add_hl_r16(virtual_cpu *cpu, const instr_operands *ops)
{
    alu_add_hl_r16(cpu, *get_r16(cpu, ops->r16));
}

static void exec_ld_r16_imm16(virtual_cpu *cpu, const instr_operands *ops)
{
    uint16_t *r16 = get_r16(cpu, ops->r16);
    *r16 = cpu->code[cpu->pc + 1] | ((uint16_t)cpu->code[cpu->pc + 2] << 8);
}

static void exec_ld_r8_imm8(virtual_cpu *cpu, const instr_operands *ops)
{
    *get_r8(cpu, ops->r8) = cpu->code[cpu->pc + 1];
}

static void exec_ld_r16mem_a(virtual_cpu *cpu, const instr_operands *ops)
{
    debug_assert(cpu->mem != NULL);

    int8_t post_hl_step = 0;
    uint16_t addr = r16mem_resolve(cpu, ops->r16, &post_hl_step);
    write_memory8(cpu->mem, addr, cpu->a);
    r16mem_apply_hl_step(cpu, post_hl_step);
}

static void exec_ld_a_r16mem(virtual_cpu *cpu, const instr_operands *ops)
{
    debug_assert(cpu->mem != NULL);

    int8_t post_hl_step = 0;
    uint16_t addr = r16mem_resolve(cpu, ops->r16, &post_hl_step);
    cpu->a = read_memory8(cpu->mem, addr);
    r16mem_apply_hl_step(cpu, post_hl_step);
}

static void exec_ld_imm16_sp(virtual_cpu *cpu)
{
    debug_assert(cpu->mem != NULL);

    uint16_t addr = cpu->code[cpu->pc + 1] | ((uint16_t)cpu->code[cpu->pc + 2] << 8);
    write_memory16(cpu->mem, addr, cpu->sp);
}

static void exec_rlca(virtual_cpu *cpu)
{
    uint8_t a = cpu->a;
    uint8_t carry = (a >> 7) & 1;
    cpu->a = (uint8_t)((a << 1) | carry);
    flags_write(cpu, 0, 0, 0, carry);
}

static void exec_rrca(virtual_cpu *cpu)
{
    uint8_t a = cpu->a;
    uint8_t carry = a & 1;
    cpu->a = (uint8_t)((a >> 1) | (carry << 7));
    flags_write(cpu, 0, 0, 0, carry);
}

static void exec_rla(virtual_cpu *cpu)
{
    uint8_t a = cpu->a;
    uint8_t old_carry = flag_get(cpu, F_MASK_C);
    uint8_t new_carry = (a >> 7) & 1;
    cpu->a = (uint8_t)((a << 1) | old_carry);
    flags_write(cpu, 0, 0, 0, new_carry);
}

static void exec_rra(virtual_cpu *cpu)
{
    uint8_t a = cpu->a;
    uint8_t old_carry = flag_get(cpu, F_MASK_C);
    uint8_t new_carry = a & 1;
    cpu->a = (uint8_t)((a >> 1) | (old_carry << 7));
    flags_write(cpu, 0, 0, 0, new_carry);
}

static void exec_daa(virtual_cpu *cpu)
{
    uint8_t adjustment = 0;
    uint8_t a = cpu->a;

    if (flag_get(cpu, F_MASK_N))
    {
        if (flag_get(cpu, F_MASK_H))
        {
            adjustment += 0x06;
        }
        if (flag_get(cpu, F_MASK_C))
        {
            adjustment += 0x60;
        }
        a -= adjustment;
        flags_write(cpu, a == 0, 1, 0, flag_get(cpu, F_MASK_C));
    }
    else
    {
        if (flag_get(cpu, F_MASK_H) || (a & 0x0F) > 0x09)
        {
            adjustment += 0x06;
        }
        uint8_t carry = 0;
        if (flag_get(cpu, F_MASK_C) || a > 0x99)
        {
            adjustment += 0x60;
            carry = 1;
        }
        a += adjustment;
        flags_write(cpu, a == 0, 0, 0, carry);
    }

    cpu->a = a;
}

static void exec_cpl(virtual_cpu *cpu)
{
    cpu->a = (uint8_t)~cpu->a;
    flags_write(cpu, flag_get(cpu, F_MASK_Z), 1, 1, flag_get(cpu, F_MASK_C));
}

static void exec_scf(virtual_cpu *cpu)
{
    flags_write(cpu, flag_get(cpu, F_MASK_Z), 0, 0, 1);
}

static void exec_ccf(virtual_cpu *cpu)
{
    uint8_t carry = !flag_get(cpu, F_MASK_C);
    flags_write(cpu, flag_get(cpu, F_MASK_Z), 0, 0, carry);
}

static void exec_jr_imm8(virtual_cpu *cpu)
{
    int8_t offset = (int8_t)cpu->code[cpu->pc + 1];
    cpu->pc += offset;
}

static void exec_jr_cond_imm8(virtual_cpu *cpu, const instr_operands *ops)
{
    int8_t offset = (int8_t)cpu->code[cpu->pc + 1];
    int taken = 0;

    switch (ops->cond)
    {
        case 0: /* NZ */
            taken = !flag_get(cpu, F_MASK_Z);
            break;
        case 1: /* Z */
            taken = flag_get(cpu, F_MASK_Z);
            break;
        case 2: /* NC */
            taken = !flag_get(cpu, F_MASK_C);
            break;
        case 3: /* C */
            taken = flag_get(cpu, F_MASK_C);
            break;
        default:
            debug_assert(0);
            break;
    }

    if (taken)
    {
        cpu->pc += offset;
        cpu->cycles += 1; /* Extra cycle when branch is taken */
    }
}

static void exec_stop(virtual_cpu *cpu)
{
    (void)cpu;
    printf("not implemented\n");
}

static void exec_ld_r8_r8(virtual_cpu *cpu, const instr_operands *ops)
{
    *get_r8(cpu, ops->r8_dest) = *get_r8(cpu, ops->r8_src);
}

static void exec_halt(virtual_cpu *cpu)
{
    (void)cpu;
    printf("halt instruction not implemented\n");
}

static void exec_alu(virtual_cpu *cpu, const instr_operands *ops)
{
    uint8_t operand = *get_r8(cpu, ops->r8);

    switch (ops->alu_op)
    {
        case 0:
            alu_add_a_r8(cpu, operand);
            break;
        case 1:
            alu_adc_a_r8(cpu, operand);
            break;
        case 2:
            alu_sub_a_r8(cpu, operand, 1);
            break;
        case 3:
            alu_sbc_a_r8(cpu, operand);
            break;
        case 4:
            alu_and_a_r8(cpu, operand);
            break;
        case 5:
            alu_xor_a_r8(cpu, operand);
            break;
        case 6:
            alu_or_a_r8(cpu, operand);
            break;
        case 7:
            alu_sub_a_r8(cpu, operand, 0);
            break;
        default:
            debug_assert(0);
            break;
    }
}

static void exec_alu_imm8(virtual_cpu *cpu, const decoded_instr *instr)
{
    uint8_t operand = cpu->code[cpu->pc + 1];

    switch (instr->id)
    {
        case INSTR_ADD_A_IMM8:
            alu_add_a_r8(cpu, operand);
            break;
        case INSTR_ADC_A_IMM8:
            alu_adc_a_r8(cpu, operand);
            break;
        case INSTR_SUB_A_IMM8:
            alu_sub_a_r8(cpu, operand, 1);
            break;
        case INSTR_SBC_A_IMM8:
            alu_sbc_a_r8(cpu, operand);
            break;
        case INSTR_AND_A_IMM8:
            alu_and_a_r8(cpu, operand);
            break;
        case INSTR_XOR_A_IMM8:
            alu_xor_a_r8(cpu, operand);
            break;
        case INSTR_OR_A_IMM8:
            alu_or_a_r8(cpu, operand);
            break;
        case INSTR_CP_A_IMM8:
            alu_sub_a_r8(cpu, operand, 0);
            break;
        default:
            debug_assert(0);
            break;
    }
}

/* CB prefix rotate/shift helpers - all write Z=0/1, N=0, H=0, C=carry */
static void exec_cb_rlc(virtual_cpu *cpu, const instr_operands *ops)
{
    uint8_t *dest = get_r8(cpu, ops->r8);
    uint8_t value = *dest;
    uint8_t carry = (value >> 7) & 1;
    uint8_t result = (uint8_t)((value << 1) | carry);
    *dest = result;
    flags_write(cpu, result == 0, 0, 0, carry);
}

static void exec_cb_rrc(virtual_cpu *cpu, const instr_operands *ops)
{
    uint8_t *dest = get_r8(cpu, ops->r8);
    uint8_t value = *dest;
    uint8_t carry = value & 1;
    uint8_t result = (uint8_t)((value >> 1) | (carry << 7));
    *dest = result;
    flags_write(cpu, result == 0, 0, 0, carry);
}

static void exec_cb_rl(virtual_cpu *cpu, const instr_operands *ops)
{
    uint8_t *dest = get_r8(cpu, ops->r8);
    uint8_t value = *dest;
    uint8_t old_carry = flag_get(cpu, F_MASK_C);
    uint8_t new_carry = (value >> 7) & 1;
    uint8_t result = (uint8_t)((value << 1) | old_carry);
    *dest = result;
    flags_write(cpu, result == 0, 0, 0, new_carry);
}

static void exec_cb_rr(virtual_cpu *cpu, const instr_operands *ops)
{
    uint8_t *dest = get_r8(cpu, ops->r8);
    uint8_t value = *dest;
    uint8_t old_carry = flag_get(cpu, F_MASK_C);
    uint8_t new_carry = value & 1;
    uint8_t result = (uint8_t)((value >> 1) | (old_carry << 7));
    *dest = result;
    flags_write(cpu, result == 0, 0, 0, new_carry);
}

static void exec_cb_sla(virtual_cpu *cpu, const instr_operands *ops)
{
    uint8_t *dest = get_r8(cpu, ops->r8);
    uint8_t value = *dest;
    uint8_t carry = (value >> 7) & 1;
    uint8_t result = (uint8_t)(value << 1);
    *dest = result;
    flags_write(cpu, result == 0, 0, 0, carry);
}

static void exec_cb_sra(virtual_cpu *cpu, const instr_operands *ops)
{
    uint8_t *dest = get_r8(cpu, ops->r8);
    uint8_t value = *dest;
    uint8_t carry = value & 1;
    uint8_t msb = value & 0x80;
    uint8_t result = (uint8_t)((value >> 1) | msb);
    *dest = result;
    flags_write(cpu, result == 0, 0, 0, carry);
}

static void exec_cb_swap(virtual_cpu *cpu, const instr_operands *ops)
{
    uint8_t *dest = get_r8(cpu, ops->r8);
    uint8_t value = *dest;
    uint8_t result = (uint8_t)(((value & 0x0F) << 4) | ((value >> 4) & 0x0F));
    *dest = result;
    flags_write(cpu, result == 0, 0, 0, 0);
}

static void exec_cb_srl(virtual_cpu *cpu, const instr_operands *ops)
{
    uint8_t *dest = get_r8(cpu, ops->r8);
    uint8_t value = *dest;
    uint8_t carry = value & 1;
    uint8_t result = (uint8_t)(value >> 1);
    *dest = result;
    flags_write(cpu, result == 0, 0, 0, carry);
}

static void exec_unknown(virtual_cpu *cpu, uint8_t opcode)
{
    (void)cpu;
    printf("unimplemented opcode %d\n", opcode);
    (void)opcode;
}

void execute_instruction(virtual_cpu *cpu, const decoded_instr *instr)
{
    switch (instr->id)
    {
        case INSTR_NOP:
            exec_nop(cpu);
            break;
        case INSTR_INC_R8:
            exec_inc_r8(cpu, &instr->ops);
            break;
        case INSTR_DEC_R8:
            exec_dec_r8(cpu, &instr->ops);
            break;
        case INSTR_INC_R16:
            exec_inc_r16(cpu, &instr->ops);
            break;
        case INSTR_DEC_R16:
            exec_dec_r16(cpu, &instr->ops);
            break;
        case INSTR_ADD_HL_R16:
            exec_add_hl_r16(cpu, &instr->ops);
            break;
        case INSTR_LD_R16_IMM16:
            exec_ld_r16_imm16(cpu, &instr->ops);
            break;
        case INSTR_LD_R8_IMM8:
            exec_ld_r8_imm8(cpu, &instr->ops);
            break;
        case INSTR_LD_R16MEM_A:
            exec_ld_r16mem_a(cpu, &instr->ops);
            break;
        case INSTR_LD_A_R16MEM:
            exec_ld_a_r16mem(cpu, &instr->ops);
            break;
        case INSTR_LD_IMM16_SP:
            exec_ld_imm16_sp(cpu);
            break;
        case INSTR_RLCA:
            exec_rlca(cpu);
            break;
        case INSTR_RRCA:
            exec_rrca(cpu);
            break;
        case INSTR_RLA:
            exec_rla(cpu);
            break;
        case INSTR_RRA:
            exec_rra(cpu);
            break;
        case INSTR_DAA:
            exec_daa(cpu);
            break;
        case INSTR_CPL:
            exec_cpl(cpu);
            break;
        case INSTR_SCF:
            exec_scf(cpu);
            break;
        case INSTR_CCF:
            exec_ccf(cpu);
            break;
        case INSTR_JR_IMM8:
            exec_jr_imm8(cpu);
            break;
        case INSTR_JR_COND_IMM8:
            exec_jr_cond_imm8(cpu, &instr->ops);
            break;
        case INSTR_STOP:
            exec_stop(cpu);
            break;
        case INSTR_LD_R8_R8:
            exec_ld_r8_r8(cpu, &instr->ops);
            break;
        case INSTR_HALT:
            exec_halt(cpu);
            break;
        case INSTR_ADD_A_R8:
        case INSTR_ADC_A_R8:
        case INSTR_SUB_A_R8:
        case INSTR_SBC_A_R8:
        case INSTR_AND_A_R8:
        case INSTR_XOR_A_R8:
        case INSTR_OR_A_R8:
        case INSTR_CP_A_R8:
            exec_alu(cpu, &instr->ops);
            break;
        case INSTR_ADD_A_IMM8:
        case INSTR_ADC_A_IMM8:
        case INSTR_SUB_A_IMM8:
        case INSTR_SBC_A_IMM8:
        case INSTR_AND_A_IMM8:
        case INSTR_XOR_A_IMM8:
        case INSTR_OR_A_IMM8:
        case INSTR_CP_A_IMM8:
            exec_alu_imm8(cpu, instr);
            break;
        case INSTR_CB_RLC:
            exec_cb_rlc(cpu, &instr->ops);
            break;
        case INSTR_CB_RRC:
            exec_cb_rrc(cpu, &instr->ops);
            break;
        case INSTR_CB_RL:
            exec_cb_rl(cpu, &instr->ops);
            break;
        case INSTR_CB_RR:
            exec_cb_rr(cpu, &instr->ops);
            break;
        case INSTR_CB_SLA:
            exec_cb_sla(cpu, &instr->ops);
            break;
        case INSTR_CB_SRA:
            exec_cb_sra(cpu, &instr->ops);
            break;
        case INSTR_CB_SWAP:
            exec_cb_swap(cpu, &instr->ops);
            break;
        case INSTR_CB_SRL:
            exec_cb_srl(cpu, &instr->ops);
            break;
        case INSTR_UNKNOWN:
            exec_unknown(cpu, cpu->code[cpu->pc]);
            break;
        default:
            debug_assert(0);
            break;
    }
}
