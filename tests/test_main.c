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
void test_jr_imm8_positive(void);
void test_jr_imm8_negative(void);
void test_jr_imm8_zero_offset(void);
void test_jr_nz_taken(void);
void test_jr_nz_not_taken(void);
void test_jr_z_taken(void);
void test_jr_z_not_taken(void);
void test_jr_nc_taken(void);
void test_jr_nc_not_taken(void);
void test_jr_c_taken(void);
void test_jr_c_not_taken(void);
void test_ld_imm16_sp(void);

/* test_cpu_block1.c */
void test_ld_r8_r8_bc(void);
void test_ld_r8_r8_self(void);
void test_ld_a_hl(void);
void test_ld_hl_a(void);

/* test_cpu_block2.c */
void test_add_a_r8(void);
void test_add_a_r8_carry(void);
void test_add_a_r8_half_carry(void);
void test_add_a_r8_no_half_carry(void);
void test_adc_a_r8_with_carry(void);
void test_adc_a_r8_without_carry(void);
void test_adc_a_r8_half_carry(void);
void test_sub_a_r8(void);
void test_sub_a_r8_borrow(void);
void test_sub_a_r8_zero(void);
void test_sub_a_r8_half_borrow(void);
void test_sbc_a_r8(void);
void test_sbc_a_r8_borrow_with_carry(void);
void test_sbc_a_r8_zero(void);
void test_and_a_r8(void);
void test_and_a_r8_zero(void);
void test_xor_a_r8(void);
void test_xor_a_r8_nonzero(void);
void test_or_a_r8(void);
void test_or_a_r8_zero(void);
void test_cp_a_r8(void);
void test_cp_a_r8_borrow(void);
void test_cp_a_r8_half_borrow(void);
void test_daa_after_add_no_adjustment(void);
void test_daa_after_add_lower_nibble_overflow(void);
void test_daa_after_add_upper_nibble_overflow(void);
void test_daa_after_add_with_half_carry(void);
void test_daa_after_sub(void);
void test_daa_after_sub_with_half_borrow(void);
void test_daa_after_sub_with_carry(void);
void test_daa_after_sub_no_adjustment(void);
void test_alu_hl_memory(void);

/* test_cpu_block3.c */
void test_add_imm8(void);
void test_add_imm8_carry(void);
void test_add_imm8_half_carry(void);
void test_add_imm8_no_half_carry(void);
void test_adc_imm8_with_carry(void);
void test_adc_imm8_without_carry(void);
void test_adc_imm8_half_carry(void);
void test_sub_imm8(void);
void test_sub_imm8_borrow(void);
void test_sub_imm8_zero(void);
void test_sub_imm8_half_borrow(void);
void test_sbc_imm8(void);
void test_sbc_imm8_borrow_with_carry(void);
void test_sbc_imm8_zero(void);
void test_and_imm8(void);
void test_and_imm8_zero(void);
void test_xor_imm8(void);
void test_xor_imm8_nonzero(void);
void test_or_imm8(void);
void test_or_imm8_zero(void);
void test_cp_imm8(void);
void test_cp_imm8_borrow(void);
void test_cp_imm8_half_borrow(void);

/* test_cpu_cb.c */
void test_cb_rlc_b(void);
void test_cb_rlc_zero(void);
void test_cb_rlc_a(void);
void test_cb_rrc_c(void);
void test_cb_rrc_zero(void);
void test_cb_rrc_no_carry(void);
void test_cb_rl_d(void);
void test_cb_rl_with_carry(void);
void test_cb_rl_no_carry(void);
void test_cb_rr_e(void);
void test_cb_rr_with_carry(void);
void test_cb_rr_no_carry(void);
void test_cb_sla_h(void);
void test_cb_sla_carry(void);
void test_cb_sla_msb_set(void);
void test_cb_sra_l(void);
void test_cb_sra_sign_extend(void);
void test_cb_sra_no_carry(void);
void test_cb_swap_a(void);
void test_cb_swap_zero(void);
void test_cb_swap_same_nibbles(void);
void test_cb_srl_b(void);
void test_cb_srl_msb_cleared(void);
void test_cb_srl_carry(void);
void test_cb_rlc_hl(void);
void test_cb_srl_hl(void);
void test_cb_instruction_cycles(void);

/* test_decode.c */
void test_decode_block0(void);
void test_decode_block1(void);
void test_decode_block2(void);
void test_decode_block3_alu_imm8(void);
void test_decode_block3_unknown(void);

/* test_mmu.c */
void test_init_memory_zeroed(void);
void test_write_read_memory8(void);
void test_write_read_memory16(void);
void test_echo_ram_write_redirect(void);
void test_echo_ram_read_redirect(void);
void test_echo_ram_bidirectional(void);
void test_echo_ram_full_range_start(void);
void test_echo_ram_full_range_end(void);
void test_non_echo_not_affected(void);
void test_work_ram_independent_from_echo(void);

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
    RUN_TEST(test_jr_imm8_positive);
    RUN_TEST(test_jr_imm8_negative);
    RUN_TEST(test_jr_imm8_zero_offset);
    RUN_TEST(test_jr_nz_taken);
    RUN_TEST(test_jr_nz_not_taken);
    RUN_TEST(test_jr_z_taken);
    RUN_TEST(test_jr_z_not_taken);
    RUN_TEST(test_jr_nc_taken);
    RUN_TEST(test_jr_nc_not_taken);
    RUN_TEST(test_jr_c_taken);
    RUN_TEST(test_jr_c_not_taken);
    RUN_TEST(test_ld_imm16_sp);

    RUN_TEST(test_ld_r8_r8_bc);
    RUN_TEST(test_ld_r8_r8_self);
    RUN_TEST(test_ld_a_hl);
    RUN_TEST(test_ld_hl_a);

    RUN_TEST(test_add_a_r8);
    RUN_TEST(test_add_a_r8_carry);
    RUN_TEST(test_add_a_r8_half_carry);
    RUN_TEST(test_add_a_r8_no_half_carry);
    RUN_TEST(test_adc_a_r8_with_carry);
    RUN_TEST(test_adc_a_r8_without_carry);
    RUN_TEST(test_adc_a_r8_half_carry);
    RUN_TEST(test_sub_a_r8);
    RUN_TEST(test_sub_a_r8_borrow);
    RUN_TEST(test_sub_a_r8_zero);
    RUN_TEST(test_sub_a_r8_half_borrow);
    RUN_TEST(test_sbc_a_r8);
    RUN_TEST(test_sbc_a_r8_borrow_with_carry);
    RUN_TEST(test_sbc_a_r8_zero);
    RUN_TEST(test_and_a_r8);
    RUN_TEST(test_and_a_r8_zero);
    RUN_TEST(test_xor_a_r8);
    RUN_TEST(test_xor_a_r8_nonzero);
    RUN_TEST(test_or_a_r8);
    RUN_TEST(test_or_a_r8_zero);
    RUN_TEST(test_cp_a_r8);
    RUN_TEST(test_cp_a_r8_borrow);
    RUN_TEST(test_cp_a_r8_half_borrow);
    RUN_TEST(test_daa_after_add_no_adjustment);
    RUN_TEST(test_daa_after_add_lower_nibble_overflow);
    RUN_TEST(test_daa_after_add_upper_nibble_overflow);
    RUN_TEST(test_daa_after_add_with_half_carry);
    RUN_TEST(test_daa_after_sub);
    RUN_TEST(test_daa_after_sub_with_half_borrow);
    RUN_TEST(test_daa_after_sub_with_carry);
    RUN_TEST(test_daa_after_sub_no_adjustment);
    RUN_TEST(test_alu_hl_memory);

    RUN_TEST(test_add_imm8);
    RUN_TEST(test_add_imm8_carry);
    RUN_TEST(test_add_imm8_half_carry);
    RUN_TEST(test_add_imm8_no_half_carry);
    RUN_TEST(test_adc_imm8_with_carry);
    RUN_TEST(test_adc_imm8_without_carry);
    RUN_TEST(test_adc_imm8_half_carry);
    RUN_TEST(test_sub_imm8);
    RUN_TEST(test_sub_imm8_borrow);
    RUN_TEST(test_sub_imm8_zero);
    RUN_TEST(test_sub_imm8_half_borrow);
    RUN_TEST(test_sbc_imm8);
    RUN_TEST(test_sbc_imm8_borrow_with_carry);
    RUN_TEST(test_sbc_imm8_zero);
    RUN_TEST(test_and_imm8);
    RUN_TEST(test_and_imm8_zero);
    RUN_TEST(test_xor_imm8);
    RUN_TEST(test_xor_imm8_nonzero);
    RUN_TEST(test_or_imm8);
    RUN_TEST(test_or_imm8_zero);
    RUN_TEST(test_cp_imm8);
    RUN_TEST(test_cp_imm8_borrow);
    RUN_TEST(test_cp_imm8_half_borrow);

    RUN_TEST(test_cb_rlc_b);
    RUN_TEST(test_cb_rlc_zero);
    RUN_TEST(test_cb_rlc_a);
    RUN_TEST(test_cb_rrc_c);
    RUN_TEST(test_cb_rrc_zero);
    RUN_TEST(test_cb_rrc_no_carry);
    RUN_TEST(test_cb_rl_d);
    RUN_TEST(test_cb_rl_with_carry);
    RUN_TEST(test_cb_rl_no_carry);
    RUN_TEST(test_cb_rr_e);
    RUN_TEST(test_cb_rr_with_carry);
    RUN_TEST(test_cb_rr_no_carry);
    RUN_TEST(test_cb_sla_h);
    RUN_TEST(test_cb_sla_carry);
    RUN_TEST(test_cb_sla_msb_set);
    RUN_TEST(test_cb_sra_l);
    RUN_TEST(test_cb_sra_sign_extend);
    RUN_TEST(test_cb_sra_no_carry);
    RUN_TEST(test_cb_swap_a);
    RUN_TEST(test_cb_swap_zero);
    RUN_TEST(test_cb_swap_same_nibbles);
    RUN_TEST(test_cb_srl_b);
    RUN_TEST(test_cb_srl_msb_cleared);
    RUN_TEST(test_cb_srl_carry);
    RUN_TEST(test_cb_rlc_hl);
    RUN_TEST(test_cb_srl_hl);
    RUN_TEST(test_cb_instruction_cycles);

    RUN_TEST(test_decode_block0);
    RUN_TEST(test_decode_block1);
    RUN_TEST(test_decode_block2);
    RUN_TEST(test_decode_block3_alu_imm8);
    RUN_TEST(test_decode_block3_unknown);

    RUN_TEST(test_init_memory_zeroed);
    RUN_TEST(test_write_read_memory8);
    RUN_TEST(test_write_read_memory16);
    RUN_TEST(test_echo_ram_write_redirect);
    RUN_TEST(test_echo_ram_read_redirect);
    RUN_TEST(test_echo_ram_bidirectional);
    RUN_TEST(test_echo_ram_full_range_start);
    RUN_TEST(test_echo_ram_full_range_end);
    RUN_TEST(test_non_echo_not_affected);
    RUN_TEST(test_work_ram_independent_from_echo);

    return UNITY_END();
}