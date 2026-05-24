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
    [INSTR_STOP] = 1,
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
