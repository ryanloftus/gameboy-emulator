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
    INSTR_ADD_A_IMM8,
    INSTR_ADC_A_IMM8,
    INSTR_SUB_A_IMM8,
    INSTR_SBC_A_IMM8,
    INSTR_AND_A_IMM8,
    INSTR_XOR_A_IMM8,
    INSTR_OR_A_IMM8,
    INSTR_CP_A_IMM8,
    /* CB prefix rotate/shift instructions */
    INSTR_CB_RLC,
    INSTR_CB_RRC,
    INSTR_CB_RL,
    INSTR_CB_RR,
    INSTR_CB_SLA,
    INSTR_CB_SRA,
    INSTR_CB_SWAP,
    INSTR_CB_SRL,
    /* CB prefix bit test / reset / set */
    INSTR_CB_BIT,
    INSTR_CB_RES,
    INSTR_CB_SET,
    /* Block 3: returns */
    INSTR_RET_NZ,
    INSTR_RET_Z,
    INSTR_RET_NC,
    INSTR_RET_C,
    INSTR_RET,
    INSTR_RETI,
    /* Block 3: jumps */
    INSTR_JP_NZ,
    INSTR_JP_Z,
    INSTR_JP_NC,
    INSTR_JP_C,
    INSTR_JP,
    INSTR_JP_HL,
    /* Block 3: calls */
    INSTR_CALL_NZ,
    INSTR_CALL_Z,
    INSTR_CALL_NC,
    INSTR_CALL_C,
    INSTR_CALL,
    /* Block 3: restarts */
    INSTR_RST,
    /* Block 3: stack */
    INSTR_POP,
    INSTR_PUSH,
    /* Block 3: high RAM and absolute loads */
    INSTR_LDH_C_A,
    INSTR_LDH_IMM8_A,
    INSTR_LD_IMM16_A,
    INSTR_LDH_A_C,
    INSTR_LDH_A_IMM8,
    INSTR_LD_A_IMM16,
    /* Block 3: SP/HL arithmetic */
    INSTR_ADD_SP_IMM8,
    INSTR_LD_HL_SP_PLUS_IMM8,
    INSTR_LD_SP_HL,
    /* Block 3: interrupt enable */
    INSTR_DI,
    INSTR_EI,
    INSTR_UNKNOWN,
    INSTR_COUNT
} instr_id;

typedef struct
{
    uint8_t r8;
    uint16_t imm16;
    uint8_t r16;
    uint8_t r8_src;
    uint8_t r8_dest;
    uint8_t alu_op;
    uint8_t cond;
    uint8_t r16stk; /* stack pair: 0=BC, 1=DE, 2=HL, 3=AF */
    uint8_t tgt3;   /* RST vector index (0-7 -> 0x00,0x08...0x38) */
    uint8_t bit;    /* Bit index for BIT/RES/SET (0-7) */
} instr_operands;

typedef struct
{
    instr_id id;
    uint8_t bytes;
    uint8_t cycles;
    instr_operands ops;
} decoded_instr;

bool decode_opcode(uint8_t opcode, decoded_instr *out);
bool decode_cb_opcode(uint8_t cb_opcode, decoded_instr *out);

#endif
