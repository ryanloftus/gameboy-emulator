#ifndef _DECODE_H_
#define _DECODE_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    INSTR_NOP,
    INSTR_INC_R8,
    INSTR_DEC_R8,
    INSTR_INC_R16,
    INSTR_DEC_R16,
    INSTR_ADD_HL_R16,
    INSTR_LD_R16_IMM16,
    INSTR_LD_R8_IMM8,
    INSTR_LD_R16MEM_A,
    INSTR_LD_A_R16MEM,
    INSTR_LD_IMM16_SP,
    INSTR_RLCA,
    INSTR_RRCA,
    INSTR_RLA,
    INSTR_RRA,
    INSTR_DAA,
    INSTR_CPL,
    INSTR_SCF,
    INSTR_CCF,
    INSTR_JR_IMM8,
    INSTR_JR_COND_IMM8,
    INSTR_STOP,
    INSTR_LD_R8_R8,
    INSTR_HALT,
    INSTR_ADD_A_R8,
    INSTR_ADC_A_R8,
    INSTR_SUB_A_R8,
    INSTR_SBC_A_R8,
    INSTR_AND_A_R8,
    INSTR_XOR_A_R8,
    INSTR_OR_A_R8,
    INSTR_CP_A_R8,
    INSTR_UNKNOWN,
    INSTR_COUNT
} instr_id;

typedef struct
{
    uint8_t r8;
    uint8_t r16;
    uint8_t r8_src;
    uint8_t r8_dest;
    uint8_t alu_op;
    uint8_t cond;
} instr_operands;

typedef struct
{
    instr_id id;
    uint8_t bytes;
    uint8_t cycles;
    instr_operands ops;
} decoded_instr;

bool decode_opcode(uint8_t opcode, decoded_instr *out);

#endif
