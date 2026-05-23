#include "unity.h"

void setUp(void) {}
void tearDown(void) {}

/* test_cpu.c (legacy) */
void test_noop(void);
void test_inc_r8(void);
void test_dec_r8(void);
void test_inc_r16(void);
void test_dec_r16(void);
void test_add_hl_r16(void);
void test_zero_flag(void);

/* test_cpu_block0.c */
void test_ld_r16_imm16(void);
void test_inc_r16_all_pairs(void);
void test_ld_r8_imm8(void);
void test_rlca(void);
void test_rrca(void);
void test_rla(void);
void test_rra(void);
void test_cpl(void);
void test_scf(void);
void test_ccf(void);
void test_inc_r8_half_carry(void);
void test_inc_r8_hl_memory(void);
void test_zero_flag_inc_dec(void);
void test_ld_r16mem_a_bc(void);
void test_ld_a_r16mem_de(void);
void test_ld_r16mem_hli(void);
void test_ld_r16mem_hld(void);
void test_ld_imm16_sp(void);

/* test_cpu_block1.c */
void test_ld_r8_r8_bc(void);
void test_ld_r8_r8_self(void);
void test_ld_a_hl(void);
void test_ld_hl_a(void);

/* test_cpu_block2.c */
void test_add_a_r8(void);
void test_add_a_r8_carry(void);
void test_adc_a_r8_with_carry(void);
void test_sub_a_r8(void);
void test_sub_a_r8_borrow(void);
void test_sbc_a_r8(void);
void test_and_a_r8(void);
void test_xor_a_r8(void);
void test_or_a_r8(void);
void test_cp_a_r8(void);
void test_cp_a_r8_borrow(void);
void test_alu_hl_memory(void);

/* test_decode.c */
void test_decode_block0(void);
void test_decode_block1(void);
void test_decode_block2(void);
void test_decode_block3_unknown(void);

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_noop);
    RUN_TEST(test_inc_r8);
    RUN_TEST(test_dec_r8);
    RUN_TEST(test_inc_r16);
    RUN_TEST(test_dec_r16);
    RUN_TEST(test_add_hl_r16);
    RUN_TEST(test_zero_flag);

    RUN_TEST(test_ld_r16_imm16);
    RUN_TEST(test_inc_r16_all_pairs);
    RUN_TEST(test_ld_r8_imm8);
    RUN_TEST(test_rlca);
    RUN_TEST(test_rrca);
    RUN_TEST(test_rla);
    RUN_TEST(test_rra);
    RUN_TEST(test_cpl);
    RUN_TEST(test_scf);
    RUN_TEST(test_ccf);
    RUN_TEST(test_inc_r8_half_carry);
    RUN_TEST(test_inc_r8_hl_memory);
    RUN_TEST(test_zero_flag_inc_dec);
    RUN_TEST(test_ld_r16mem_a_bc);
    RUN_TEST(test_ld_a_r16mem_de);
    RUN_TEST(test_ld_r16mem_hli);
    RUN_TEST(test_ld_r16mem_hld);
    RUN_TEST(test_ld_imm16_sp);

    RUN_TEST(test_ld_r8_r8_bc);
    RUN_TEST(test_ld_r8_r8_self);
    RUN_TEST(test_ld_a_hl);
    RUN_TEST(test_ld_hl_a);

    RUN_TEST(test_add_a_r8);
    RUN_TEST(test_add_a_r8_carry);
    RUN_TEST(test_adc_a_r8_with_carry);
    RUN_TEST(test_sub_a_r8);
    RUN_TEST(test_sub_a_r8_borrow);
    RUN_TEST(test_sbc_a_r8);
    RUN_TEST(test_and_a_r8);
    RUN_TEST(test_xor_a_r8);
    RUN_TEST(test_or_a_r8);
    RUN_TEST(test_cp_a_r8);
    RUN_TEST(test_cp_a_r8_borrow);
    RUN_TEST(test_alu_hl_memory);

    RUN_TEST(test_decode_block0);
    RUN_TEST(test_decode_block1);
    RUN_TEST(test_decode_block2);
    RUN_TEST(test_decode_block3_unknown);

    return UNITY_END();
}
