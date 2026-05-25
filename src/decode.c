#include "decode.h"

#include <stddef.h>

typedef struct
{
    uint8_t bitmask;
    uint8_t pattern;
    instr_id id;
} opcode_pattern;

static const uint8_t instr_bytes[INSTR_COUNT] =
{
    [INSTR_NOP] = 1,
    [INSTR_INC_R8] = 1,
    [INSTR_DEC_R8] = 1,
    [INSTR_INC_R16] = 1,
    [INSTR_DEC_R16] = 1,
    [INSTR_ADD_HL_R16] = 1,
    [INSTR_LD_R16_IMM16] = 3,
    [INSTR_LD_R8_IMM8] = 2,
    [INSTR_LD_R16MEM_A] = 1,
    [INSTR_LD_A_R16MEM] = 1,
    [INSTR_LD_IMM16_SP] = 3,
    [INSTR_RLCA] = 1,
    [INSTR_RRCA] = 1,
    [INSTR_RLA] = 1,
    [INSTR_RRA] = 1,
    [INSTR_DAA] = 1,
    [INSTR_CPL] = 1,
    [INSTR_SCF] = 1,
    [INSTR_CCF] = 1,
    [INSTR_JR_IMM8] = 2,
    [INSTR_JR_COND_IMM8] = 2,
    [INSTR_STOP] = 2,
    [INSTR_LD_R8_R8] = 1,
    [INSTR_HALT] = 1,
    [INSTR_ADD_A_R8] = 1,
    [INSTR_ADC_A_R8] = 1,
    [INSTR_SUB_A_R8] = 1,
    [INSTR_SBC_A_R8] = 1,
    [INSTR_AND_A_R8] = 1,
    [INSTR_XOR_A_R8] = 1,
    [INSTR_OR_A_R8] = 1,
    [INSTR_CP_A_R8] = 1,
    [INSTR_ADD_A_IMM8] = 2,
    [INSTR_ADC_A_IMM8] = 2,
    [INSTR_SUB_A_IMM8] = 2,
    [INSTR_SBC_A_IMM8] = 2,
    [INSTR_AND_A_IMM8] = 2,
    [INSTR_XOR_A_IMM8] = 2,
    [INSTR_OR_A_IMM8] = 2,
    [INSTR_CP_A_IMM8] = 2,
    [INSTR_CB_RLC] = 2,
    [INSTR_CB_RRC] = 2,
    [INSTR_CB_RL] = 2,
    [INSTR_CB_RR] = 2,
    [INSTR_CB_SLA] = 2,
    [INSTR_CB_SRA] = 2,
    [INSTR_CB_SWAP] = 2,
    [INSTR_CB_SRL] = 2,
    [INSTR_CB_BIT] = 2,
    [INSTR_CB_RES] = 2,
    [INSTR_CB_SET] = 2,
    /* Block 3: returns */
    [INSTR_RET_NZ] = 1,
    [INSTR_RET_Z] = 1,
    [INSTR_RET_NC] = 1,
    [INSTR_RET_C] = 1,
    [INSTR_RET] = 1,
    [INSTR_RETI] = 1,
    /* Block 3: jumps */
    [INSTR_JP_NZ] = 3,
    [INSTR_JP_Z] = 3,
    [INSTR_JP_NC] = 3,
    [INSTR_JP_C] = 3,
    [INSTR_JP] = 3,
    [INSTR_JP_HL] = 1,
    /* Block 3: calls */
    [INSTR_CALL_NZ] = 3,
    [INSTR_CALL_Z] = 3,
    [INSTR_CALL_NC] = 3,
    [INSTR_CALL_C] = 3,
    [INSTR_CALL] = 3,
    /* Block 3: restarts */
    [INSTR_RST] = 1,
    /* Block 3: stack */
    [INSTR_POP] = 1,
    [INSTR_PUSH] = 1,
    /* Block 3: high RAM and absolute loads */
    [INSTR_LDH_C_A] = 1,
    [INSTR_LDH_IMM8_A] = 2,
    [INSTR_LD_IMM16_A] = 3,
    [INSTR_LDH_A_C] = 1,
    [INSTR_LDH_A_IMM8] = 2,
    [INSTR_LD_A_IMM16] = 3,
    /* Block 3: SP/HL arithmetic */
    [INSTR_ADD_SP_IMM8] = 2,
    [INSTR_LD_HL_SP_PLUS_IMM8] = 2,
    [INSTR_LD_SP_HL] = 1,
    /* Block 3: interrupt enable */
    [INSTR_DI] = 1,
    [INSTR_EI] = 1,
    [INSTR_UNKNOWN] = 1,
};

static const uint8_t instr_cycles[INSTR_COUNT] =
{
    [INSTR_NOP] = 1,
    [INSTR_INC_R8] = 1,
    [INSTR_DEC_R8] = 1,
    [INSTR_INC_R16] = 2,
    [INSTR_DEC_R16] = 2,
    [INSTR_ADD_HL_R16] = 2,
    [INSTR_LD_R16_IMM16] = 3,
    [INSTR_LD_R8_IMM8] = 2,
    [INSTR_LD_R16MEM_A] = 2,
    [INSTR_LD_A_R16MEM] = 2,
    [INSTR_LD_IMM16_SP] = 5,
    [INSTR_RLCA] = 1,
    [INSTR_RRCA] = 1,
    [INSTR_RLA] = 1,
    [INSTR_RRA] = 1,
    [INSTR_DAA] = 1,
    [INSTR_CPL] = 1,
    [INSTR_SCF] = 1,
    [INSTR_CCF] = 1,
    [INSTR_JR_IMM8] = 3,
    [INSTR_JR_COND_IMM8] = 2,
    [INSTR_STOP] = 0,
    [INSTR_LD_R8_R8] = 1,
    [INSTR_HALT] = 0,
    [INSTR_ADD_A_R8] = 1,
    [INSTR_ADC_A_R8] = 1,
    [INSTR_SUB_A_R8] = 1,
    [INSTR_SBC_A_R8] = 1,
    [INSTR_AND_A_R8] = 1,
    [INSTR_XOR_A_R8] = 1,
    [INSTR_OR_A_R8] = 1,
    [INSTR_CP_A_R8] = 1,
    [INSTR_ADD_A_IMM8] = 2,
    [INSTR_ADC_A_IMM8] = 2,
    [INSTR_SUB_A_IMM8] = 2,
    [INSTR_SBC_A_IMM8] = 2,
    [INSTR_AND_A_IMM8] = 2,
    [INSTR_XOR_A_IMM8] = 2,
    [INSTR_OR_A_IMM8] = 2,
    [INSTR_CP_A_IMM8] = 2,
    [INSTR_CB_RLC] = 2,
    [INSTR_CB_RRC] = 2,
    [INSTR_CB_RL] = 2,
    [INSTR_CB_RR] = 2,
    [INSTR_CB_SLA] = 2,
    [INSTR_CB_SRA] = 2,
    [INSTR_CB_SWAP] = 2,
    [INSTR_CB_SRL] = 2,
    [INSTR_CB_BIT] = 2,
    [INSTR_CB_RES] = 2,
    [INSTR_CB_SET] = 2,
    /* Block 3: returns */
    [INSTR_RET_NZ] = 2,
    [INSTR_RET_Z] = 2,
    [INSTR_RET_NC] = 2,
    [INSTR_RET_C] = 2,
    [INSTR_RET] = 4,
    [INSTR_RETI] = 4,
    /* Block 3: jumps */
    [INSTR_JP_NZ] = 3,
    [INSTR_JP_Z] = 3,
    [INSTR_JP_NC] = 3,
    [INSTR_JP_C] = 3,
    [INSTR_JP] = 4,
    [INSTR_JP_HL] = 1,
    /* Block 3: calls */
    [INSTR_CALL_NZ] = 3,
    [INSTR_CALL_Z] = 3,
    [INSTR_CALL_NC] = 3,
    [INSTR_CALL_C] = 3,
    [INSTR_CALL] = 6,
    /* Block 3: restarts */
    [INSTR_RST] = 4,
    /* Block 3: stack */
    [INSTR_POP] = 3,
    [INSTR_PUSH] = 4,
    /* Block 3: high RAM and absolute loads */
    [INSTR_LDH_C_A] = 2,
    [INSTR_LDH_IMM8_A] = 3,
    [INSTR_LD_IMM16_A] = 4,
    [INSTR_LDH_A_C] = 2,
    [INSTR_LDH_A_IMM8] = 3,
    [INSTR_LD_A_IMM16] = 4,
    /* Block 3: SP/HL arithmetic */
    [INSTR_ADD_SP_IMM8] = 4,
    [INSTR_LD_HL_SP_PLUS_IMM8] = 3,
    [INSTR_LD_SP_HL] = 2,
    /* Block 3: interrupt enable */
    [INSTR_DI] = 1,
    [INSTR_EI] = 1,
    [INSTR_UNKNOWN] = 0,
};

/* Order: first match wins — list more specific patterns before general ones. */
static const opcode_pattern block_zero_patterns[] =
{
    {0xFF, 0x00, INSTR_NOP},
    {0xFF, 0x08, INSTR_LD_IMM16_SP},
    {0xFF, 0x10, INSTR_STOP},
    {0xFF, 0x18, INSTR_JR_IMM8},
    {0xFF, 0x07, INSTR_RLCA},
    {0xFF, 0x0F, INSTR_RRCA},
    {0xFF, 0x17, INSTR_RLA},
    {0xFF, 0x1F, INSTR_RRA},
    {0xFF, 0x27, INSTR_DAA},
    {0xFF, 0x2F, INSTR_CPL},
    {0xFF, 0x37, INSTR_SCF},
    {0xFF, 0x3F, INSTR_CCF},
    {0xC7, 0x00, INSTR_JR_COND_IMM8},
    {0xCF, 0x01, INSTR_LD_R16_IMM16},
    {0xCF, 0x0A, INSTR_LD_A_R16MEM},
    {0xCF, 0x02, INSTR_LD_R16MEM_A},
    {0xCF, 0x03, INSTR_INC_R16},
    {0xCF, 0x0B, INSTR_DEC_R16},
    {0xCF, 0x09, INSTR_ADD_HL_R16},
    {0xC7, 0x06, INSTR_LD_R8_IMM8},
    {0xC7, 0x04, INSTR_INC_R8},
    {0xC7, 0x05, INSTR_DEC_R8},
};

static void fill_decoded(decoded_instr *out, instr_id id, const instr_operands *ops)
{
    out->id = id;
    out->bytes = instr_bytes[id];
    out->cycles = instr_cycles[id];
    if (ops != NULL)
    {
        out->ops = *ops;
    }
    else
    {
        out->ops = (instr_operands){0};
    }
}

static bool decode_block_zero(uint8_t opcode, decoded_instr *out)
{
    for (size_t i = 0; i < sizeof(block_zero_patterns) / sizeof(block_zero_patterns[0]); ++i)
    {
        const opcode_pattern *p = &block_zero_patterns[i];
        if ((opcode & p->bitmask) != p->pattern)
        {
            continue;
        }

        instr_operands ops = {0};

        switch (p->id)
        {
            case INSTR_LD_R16_IMM16:
            case INSTR_INC_R16:
            case INSTR_DEC_R16:
            case INSTR_ADD_HL_R16:
            case INSTR_LD_R16MEM_A:
            case INSTR_LD_A_R16MEM:
                ops.r16 = (opcode >> 4) & 0b11;
                break;
            case INSTR_INC_R8:
            case INSTR_DEC_R8:
            case INSTR_LD_R8_IMM8:
                ops.r8 = (opcode >> 3) & 0b111;
                break;
            case INSTR_JR_COND_IMM8:
                ops.cond = (opcode >> 3) & 0b11;
                break;
            default:
                break;
        }

        fill_decoded(out, p->id, &ops);
        return true;
    }

    return false;
}

static bool decode_block_one(uint8_t opcode, decoded_instr *out)
{
    if (opcode == 0x76)
    {
        fill_decoded(out, INSTR_HALT, NULL);
        return true;
    }

    instr_operands ops =
    {
        .r8_src = opcode & 0b111,
        .r8_dest = (opcode >> 3) & 0b111,
    };
    fill_decoded(out, INSTR_LD_R8_R8, &ops);
    return true;
}

/* Block 3 opcode patterns (bits 7-6 = 11, excludes ALU immediate handled separately) */
typedef struct
{
    uint8_t bitmask;
    uint8_t pattern;
    instr_id id;
} block3_pattern;

static const block3_pattern block3_main_patterns[] =
{
    /* Returns */
    {0xFF, 0xC0, INSTR_RET_NZ},
    {0xFF, 0xC8, INSTR_RET_Z},
    {0xFF, 0xD0, INSTR_RET_NC},
    {0xFF, 0xD8, INSTR_RET_C},
    {0xFF, 0xC9, INSTR_RET},
    {0xFF, 0xD9, INSTR_RETI},
    /* Jumps */
    {0xFF, 0xC2, INSTR_JP_NZ},
    {0xFF, 0xCA, INSTR_JP_Z},
    {0xFF, 0xD2, INSTR_JP_NC},
    {0xFF, 0xDA, INSTR_JP_C},
    {0xFF, 0xC3, INSTR_JP},
    {0xFF, 0xE9, INSTR_JP_HL},
    /* Calls */
    {0xFF, 0xC4, INSTR_CALL_NZ},
    {0xFF, 0xCC, INSTR_CALL_Z},
    {0xFF, 0xD4, INSTR_CALL_NC},
    {0xFF, 0xDC, INSTR_CALL_C},
    {0xFF, 0xCD, INSTR_CALL},
    /* Stack */
    {0xFF, 0xE2, INSTR_LDH_C_A},
    {0xFF, 0xE0, INSTR_LDH_IMM8_A},
    {0xFF, 0xEA, INSTR_LD_IMM16_A},
    {0xFF, 0xF2, INSTR_LDH_A_C},
    {0xFF, 0xF0, INSTR_LDH_A_IMM8},
    {0xFF, 0xFA, INSTR_LD_A_IMM16},
    /* SP/HL */
    {0xFF, 0xE8, INSTR_ADD_SP_IMM8},
    {0xFF, 0xF8, INSTR_LD_HL_SP_PLUS_IMM8},
    {0xFF, 0xF9, INSTR_LD_SP_HL},
    /* Interrupt */
    {0xFF, 0xF3, INSTR_DI},
    {0xFF, 0xFB, INSTR_EI},
};

/* RST vectors: opcodes 0xC7, 0xCF, 0xD7, 0xDF, 0xE7, 0xEF, 0xF7, 0xFF */
static const uint8_t RST_TGT3_MASK = 0xC7;
static const uint8_t RST_TGT3_PATTERN = 0xC7;

/* RST tgt3 values: 0->0x00, 1->0x08, 2->0x10, 3->0x18, 4->0x20, 5->0x28, 6->0x30, 7->0x38 */
static const uint16_t rst_vectors[] = {0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38};

static bool decode_block_three(uint8_t opcode, decoded_instr *out)
{
    /* ALU immediate: opcode bits 3-5 = alu_op, bits 7-6 = 11, bits 2-0 = 110 */
    static const uint8_t mask  = 0xC7;  /* 11000111 */
    static const uint8_t pattern = 0xC6; /* 11000110 */

    if ((opcode & mask) == pattern)
    {
        uint8_t alu_op = (opcode >> 3) & 0b111;

        static const instr_id alu_imm_ids[] =
        {
            INSTR_ADD_A_IMM8,
            INSTR_ADC_A_IMM8,
            INSTR_SUB_A_IMM8,
            INSTR_SBC_A_IMM8,
            INSTR_AND_A_IMM8,
            INSTR_XOR_A_IMM8,
            INSTR_OR_A_IMM8,
            INSTR_CP_A_IMM8,
        };

        fill_decoded(out, alu_imm_ids[alu_op], NULL);
        return true;
    }

    /* RST instructions: 0xC7, 0xCF, 0xD7, 0xDF, 0xE7, 0xEF, 0xF7, 0xFF */
    /* This also covers 0xC7 (RST 0), so must come AFTER ALU immediate check */
    if ((opcode & RST_TGT3_MASK) == RST_TGT3_PATTERN)
    {
        /* RST opcodes have bits 3-5 = tgt3 index */
        uint8_t tgt3 = (opcode >> 3) & 0b111;
        /* But we also need to exclude ALU imms - those have bits 2-0 = 110 */
        /* RST has bits 2-0 as 111, so no conflict */
        instr_operands ops = {0};
        ops.tgt3 = tgt3;
        fill_decoded(out, INSTR_RST, &ops);
        return true;
    }

    /* POP/PUSH: opcode bits 4-3 = r16stk, with bits 5,2,1,0 as fixed pattern */
    /* POP: 11xx0001 -> mask 0xCF, pattern 0xC1 */
    /* PUSH: 11xx0101 -> mask 0xCF, pattern 0xC5 */
    if ((opcode & 0xCF) == 0xC1)
    {
        instr_operands ops = {0};
        ops.r16stk = (opcode >> 4) & 0b11;
        fill_decoded(out, INSTR_POP, &ops);
        return true;
    }
    if ((opcode & 0xCF) == 0xC5)
    {
        instr_operands ops = {0};
        ops.r16stk = (opcode >> 4) & 0b11;
        fill_decoded(out, INSTR_PUSH, &ops);
        return true;
    }

    /* Main patterns table */
    for (size_t i = 0; i < sizeof(block3_main_patterns) / sizeof(block3_main_patterns[0]); ++i)
    {
        const block3_pattern *p = &block3_main_patterns[i];
        if ((opcode & p->bitmask) != p->pattern)
        {
            continue;
        }

        instr_operands ops = {0};
        /* Set cond field for conditional instructions */
        switch (p->id)
        {
            case INSTR_RET_NZ:
            case INSTR_JP_NZ:
            case INSTR_CALL_NZ:
                ops.cond = 0;
                break;
            case INSTR_RET_Z:
            case INSTR_JP_Z:
            case INSTR_CALL_Z:
                ops.cond = 1;
                break;
            case INSTR_RET_NC:
            case INSTR_JP_NC:
            case INSTR_CALL_NC:
                ops.cond = 2;
                break;
            case INSTR_RET_C:
            case INSTR_JP_C:
            case INSTR_CALL_C:
                ops.cond = 3;
                break;
            default:
                break;
        }
        fill_decoded(out, p->id, &ops);
        return true;
    }

    return false;
}

static bool decode_block_two(uint8_t opcode, decoded_instr *out)
{
    instr_operands ops =
    {
        .r8 = opcode & 0b111,
        .alu_op = (opcode >> 3) & 0b111,
    };

    static const instr_id alu_ids[] =
    {
        INSTR_ADD_A_R8,
        INSTR_ADC_A_R8,
        INSTR_SUB_A_R8,
        INSTR_SBC_A_R8,
        INSTR_AND_A_R8,
        INSTR_XOR_A_R8,
        INSTR_OR_A_R8,
        INSTR_CP_A_R8,
    };

    fill_decoded(out, alu_ids[ops.alu_op], &ops);
    return true;
}

bool decode_opcode(uint8_t opcode, decoded_instr *out)
{
    if (out == NULL)
    {
        return false;
    }

    uint8_t block = (opcode & 0b11000000) >> 6;

    switch (block)
    {
        case 0:
            if (decode_block_zero(opcode, out))
            {
                return true;
            }
            break;
        case 1:
            return decode_block_one(opcode, out);
        case 2:
            return decode_block_two(opcode, out);
        case 3:
            if (decode_block_three(opcode, out))
            {
                return true;
            }
            break;
        default:
            break;
    }

    fill_decoded(out, INSTR_UNKNOWN, NULL);
    return false;
}

/* CB prefix decode: opcode bits 7-6 select family, bits 5-3 = operation/bit, bits 2-0 = r8 */
bool decode_cb_opcode(uint8_t cb_opcode, decoded_instr *out)
{
    if (out == NULL)
    {
        return false;
    }

    instr_operands ops = {0};
    ops.r8 = cb_opcode & 0b111;
    uint8_t family = (cb_opcode >> 6) & 0b11;
    uint8_t op_type = (cb_opcode >> 3) & 0b111;

    static const instr_id cb_rotate_shift_ids[] =
    {
        INSTR_CB_RLC,
        INSTR_CB_RRC,
        INSTR_CB_RL,
        INSTR_CB_RR,
        INSTR_CB_SLA,
        INSTR_CB_SRA,
        INSTR_CB_SWAP,
        INSTR_CB_SRL,
    };

    switch (family)
    {
        case 0: /* Rotate/shift: opcodes 0x00-0x3F */
            if (op_type < 8)
            {
                fill_decoded(out, cb_rotate_shift_ids[op_type], &ops);
                return true;
            }
            break;
        case 1: /* BIT: opcodes 0x40-0x7F */
            ops.bit = op_type;
            fill_decoded(out, INSTR_CB_BIT, &ops);
            return true;
        case 2: /* RES: opcodes 0x80-0xBF */
            ops.bit = op_type;
            fill_decoded(out, INSTR_CB_RES, &ops);
            return true;
        case 3: /* SET: opcodes 0xC0-0xFF */
            ops.bit = op_type;
            fill_decoded(out, INSTR_CB_SET, &ops);
            return true;
    }

    fill_decoded(out, INSTR_UNKNOWN, NULL);
    return false;
}
