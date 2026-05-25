#include "unity.h"
#include "decode.h"

#include <stdint.h>

typedef struct
{
    uint8_t opcode;
    instr_id id;
    uint8_t bytes;
    uint8_t r8;
    uint8_t r16;
    uint8_t r8_src;
    uint8_t r8_dest;
    uint8_t alu_op;
} decode_case;

static void run_decode_case(const decode_case *c)
{
    decoded_instr dec;
    TEST_ASSERT_TRUE(decode_opcode(c->opcode, &dec));
    TEST_ASSERT_EQUAL_INT(c->id, dec.id);
    TEST_ASSERT_EQUAL_UINT8(c->bytes, dec.bytes);
    if (c->r8 != 0xFF)
    {
        TEST_ASSERT_EQUAL_UINT8(c->r8, dec.ops.r8);
    }
    if (c->r16 != 0xFF)
    {
        TEST_ASSERT_EQUAL_UINT8(c->r16, dec.ops.r16);
    }
    if (c->r8_src != 0xFF)
    {
        TEST_ASSERT_EQUAL_UINT8(c->r8_src, dec.ops.r8_src);
    }
    if (c->r8_dest != 0xFF)
    {
        TEST_ASSERT_EQUAL_UINT8(c->r8_dest, dec.ops.r8_dest);
    }
    if (c->alu_op != 0xFF)
    {
        TEST_ASSERT_EQUAL_UINT8(c->alu_op, dec.ops.alu_op);
    }
}

void test_decode_block0(void)
{
    const decode_case cases[] =
    {
        {0x00, INSTR_NOP, 1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        {0x0C, INSTR_INC_R8, 1, 1, 0xFF, 0xFF, 0xFF, 0xFF},
        {0x01, INSTR_LD_R16_IMM16, 3, 0xFF, 0, 0xFF, 0xFF, 0xFF},
        {0x06, INSTR_LD_R8_IMM8, 2, 0, 0xFF, 0xFF, 0xFF, 0xFF},
        {0x07, INSTR_RLCA, 1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        {0x02, INSTR_LD_R16MEM_A, 1, 0xFF, 0, 0xFF, 0xFF, 0xFF},
        {0x0A, INSTR_LD_A_R16MEM, 1, 0xFF, 0, 0xFF, 0xFF, 0xFF},
        {0x22, INSTR_LD_R16MEM_A, 1, 0xFF, 2, 0xFF, 0xFF, 0xFF},
        {0x08, INSTR_LD_IMM16_SP, 3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i)
    {
        run_decode_case(&cases[i]);
    }
}

void test_decode_block1(void)
{
    const decode_case cases[] =
    {
        {0x76, INSTR_HALT, 1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        {0x41, INSTR_LD_R8_R8, 1, 0xFF, 0xFF, 1, 0, 0xFF},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i)
    {
        run_decode_case(&cases[i]);
    }
}

void test_decode_block2(void)
{
    const decode_case cases[] =
    {
        {0x80, INSTR_ADD_A_R8, 1, 0, 0xFF, 0xFF, 0xFF, 0},
        {0x89, INSTR_ADC_A_R8, 1, 1, 0xFF, 0xFF, 0xFF, 1},
        {0x90, INSTR_SUB_A_R8, 1, 0, 0xFF, 0xFF, 0xFF, 2},
        {0xBE, INSTR_CP_A_R8, 1, 6, 0xFF, 0xFF, 0xFF, 7},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i)
    {
        run_decode_case(&cases[i]);
    }
}

void test_decode_block3_alu_imm8(void)
{
    /* ALU immediate opcodes: bits 7-6 = 11, bits 2-0 = 110, alu_op in bits 3-5 */
    const decode_case cases[] =
    {
        {0xC6, INSTR_ADD_A_IMM8, 2, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* ADD A, imm8 */
        {0xCE, INSTR_ADC_A_IMM8, 2, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* ADC A, imm8 */
        {0xD6, INSTR_SUB_A_IMM8, 2, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* SUB A, imm8 */
        {0xDE, INSTR_SBC_A_IMM8, 2, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* SBC A, imm8 */
        {0xE6, INSTR_AND_A_IMM8, 2, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* AND A, imm8 */
        {0xEE, INSTR_XOR_A_IMM8, 2, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* XOR A, imm8 */
        {0xF6, INSTR_OR_A_IMM8, 2, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* OR A, imm8 */
        {0xFE, INSTR_CP_A_IMM8, 2, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* CP A, imm8 */
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i)
    {
        run_decode_case(&cases[i]);
    }
}

void test_decode_block3_unknown(void)
{
    /* All 256 CB opcodes are valid, so no unknown CB case.
       Test specific block 3 decodes instead. */

    /* RST 0x38 (opcode 0xFF) */
    decoded_instr dec;
    TEST_ASSERT_TRUE(decode_opcode(0xFF, &dec));
    TEST_ASSERT_EQUAL_INT(INSTR_RST, dec.id);
    TEST_ASSERT_EQUAL_UINT8(7, dec.ops.tgt3); /* RST 0x38 */

    /* RST 0x00 (opcode 0xC7) */
    TEST_ASSERT_TRUE(decode_opcode(0xC7, &dec));
    TEST_ASSERT_EQUAL_INT(INSTR_RST, dec.id);
    TEST_ASSERT_EQUAL_UINT8(0, dec.ops.tgt3); /* RST 0x00 */

    /* CB prefix SET 7, A (opcode 0xFF in CB space: bits 7-6=11=SET, bits 5-3=111=bit7, bits 2-0=111=A) */
    decoded_instr cb_dec;
    TEST_ASSERT_TRUE(decode_cb_opcode(0xFF, &cb_dec));
    TEST_ASSERT_EQUAL_INT(INSTR_CB_SET, cb_dec.id);
    TEST_ASSERT_EQUAL_UINT8(7, cb_dec.ops.r8); /* A register */
    TEST_ASSERT_EQUAL_UINT8(7, cb_dec.ops.bit); /* bit 7 */
}
