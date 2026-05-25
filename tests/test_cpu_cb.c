#include "cpu_test_util.h"

#include "mmu.h"

#include <stdint.h>

/*
 * CB prefix opcodes:
 * Bits 7-6: operation type (00=RLC, 01=RRC, 10=RL, 11=RR, 100=SLA, 101=SRA, 110=SWAP, 111=SRL)
 * Bits 5-3: r8 id (same as block 1)
 * Bits 2-0: r8 id
 *
 * CB prefix is 0xCB, second byte picks the operation.
 * r8 ids: 0=B, 1=C, 2=D, 3=E, 4=H, 5=L, 6=[HL], 7=A
 */

/* === RLC r8 — Rotate left through carry (Z=result==0, N=0, H=0, C=old bit 7) === */
void test_cb_rlc_b(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x00};

    cpu_test_reset(&cpu, NULL, code);
    cpu.b = 0x85; /* 10000101 -> carry=1, result=00001011 */
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x0B, cpu.b);
    assert_flags(&cpu, 0, 0, 0, 1);
    TEST_ASSERT_EQUAL_UINT16(2, cpu.pc);
}

void test_cb_rlc_zero(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x00};

    cpu_test_reset(&cpu, NULL, code);
    cpu.b = 0x00;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.b);
    assert_flags(&cpu, 1, 0, 0, 0);
}

void test_cb_rlc_a(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x07};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x80;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x01, cpu.a);
    assert_flags(&cpu, 0, 0, 0, 1);
}

/* === RRC r8 — Rotate right through carry (Z=result==0, N=0, H=0, C=old bit 0) === */
void test_cb_rrc_c(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x09};

    cpu_test_reset(&cpu, NULL, code);
    cpu.c = 0xA1; /* 10100001 -> carry=1, result=11010000 */
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xD0, cpu.c);
    assert_flags(&cpu, 0, 0, 0, 1);
}

void test_cb_rrc_zero(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x09};

    cpu_test_reset(&cpu, NULL, code);
    cpu.c = 0x00;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.c);
    assert_flags(&cpu, 1, 0, 0, 0);
}

void test_cb_rrc_no_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x08};

    cpu_test_reset(&cpu, NULL, code);
    cpu.b = 0x02; /* 00000010 -> carry=0, result=00000001 */
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x01, cpu.b);
    assert_flags(&cpu, 0, 0, 0, 0);
}

/* === RL r8 — Rotate left through carry (Z=result==0, N=0, H=0, C=old bit 7) === */
void test_cb_rl_d(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x12};

    cpu_test_reset(&cpu, NULL, code);
    cpu.d = 0x80; /* 10000000, C=0 -> result=00000000, new C=1 */
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.d);
    assert_flags(&cpu, 1, 0, 0, 1);
}

void test_cb_rl_with_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x12};

    cpu_test_reset(&cpu, NULL, code);
    cpu.d = 0x01; /* 00000001, with C=1 -> result=00000011, new C=0 */
    cpu.f = F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x03, cpu.d);
    assert_flags(&cpu, 0, 0, 0, 0);
}

void test_cb_rl_no_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x12};

    cpu_test_reset(&cpu, NULL, code);
    cpu.d = 0x40; /* 01000000, C=0 -> result=10000000, new C=0 */
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x80, cpu.d);
    assert_flags(&cpu, 0, 0, 0, 0);
}

/* === RR r8 — Rotate right through carry (Z=result==0, N=0, H=0, C=old bit 0) === */
void test_cb_rr_e(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x1B};

    cpu_test_reset(&cpu, NULL, code);
    cpu.e = 0x01; /* 00000001, C=0 -> result=00000000, new C=1 */
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.e);
    assert_flags(&cpu, 1, 0, 0, 1);
}

void test_cb_rr_with_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x1B};

    cpu_test_reset(&cpu, NULL, code);
    cpu.e = 0x80; /* 10000000, with C=1 -> result=11000000, new C=0 */
    cpu.f = F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xC0, cpu.e);
    assert_flags(&cpu, 0, 0, 0, 0);
}

void test_cb_rr_no_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x1A};

    cpu_test_reset(&cpu, NULL, code);
    cpu.d = 0x02; /* 00000010, C=0 -> result=00000001, new C=0 */
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x01, cpu.d);
    assert_flags(&cpu, 0, 0, 0, 0);
}

/* === SLA r8 — Shift left arithmetic (Z=result==0, N=0, H=0, C=old bit 7) === */
void test_cb_sla_h(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x24};

    cpu_test_reset(&cpu, NULL, code);
    cpu.h = 0x80; /* 10000000 -> result=00000000, C=1 */
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.h);
    assert_flags(&cpu, 1, 0, 0, 1);
}

void test_cb_sla_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x24};

    cpu_test_reset(&cpu, NULL, code);
    cpu.h = 0x01; /* 00000001 -> result=00000010, C=0 */
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x02, cpu.h);
    assert_flags(&cpu, 0, 0, 0, 0);
}

void test_cb_sla_msb_set(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x20};

    cpu_test_reset(&cpu, NULL, code);
    cpu.b = 0xFF; /* 11111111 -> result=11111110, C=1 */
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xFE, cpu.b);
    assert_flags(&cpu, 0, 0, 0, 1);
}

/* === SRA r8 — Shift right arithmetic (Z=result==0, N=0, H=0, C=old bit 0) === */
void test_cb_sra_l(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x2D};

    cpu_test_reset(&cpu, NULL, code);
    cpu.l = 0x01; /* 00000001 -> result=00000000, C=1 */
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.l);
    assert_flags(&cpu, 1, 0, 0, 1);
}

void test_cb_sra_sign_extend(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x2D};

    cpu_test_reset(&cpu, NULL, code);
    cpu.l = 0x83; /* 10000011 -> result=11000001, C=1 */
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.l);
    assert_flags(&cpu, 0, 0, 0, 1);
}

void test_cb_sra_no_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x28};

    cpu_test_reset(&cpu, NULL, code);
    cpu.b = 0x02; /* 00000010 -> result=00000001, C=0 */
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x01, cpu.b);
    assert_flags(&cpu, 0, 0, 0, 0);
}

/* === SWAP r8 — Swap nibbles (Z=result==0, N=0, H=0, C=0) === */
void test_cb_swap_a(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x37};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0xAB;
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xBA, cpu.a);
    assert_flags(&cpu, 0, 0, 0, 0);
}

void test_cb_swap_zero(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x37};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x00;
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.a);
    assert_flags(&cpu, 1, 0, 0, 0);
}

void test_cb_swap_same_nibbles(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x30};

    cpu_test_reset(&cpu, NULL, code);
    cpu.b = 0x33;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x33, cpu.b);
    assert_flags(&cpu, 0, 0, 0, 0);
}

/* === SRL r8 — Shift right logical (Z=result==0, N=0, H=0, C=old bit 0) === */
void test_cb_srl_b(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x38};

    cpu_test_reset(&cpu, NULL, code);
    cpu.b = 0x01; /* 00000001 -> result=00000000, C=1 */
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.b);
    assert_flags(&cpu, 1, 0, 0, 1);
}

void test_cb_srl_msb_cleared(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x38};

    cpu_test_reset(&cpu, NULL, code);
    cpu.b = 0x80; /* 10000000 -> result=01000000, C=0 */
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x40, cpu.b);
    assert_flags(&cpu, 0, 0, 0, 0);
}

void test_cb_srl_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x3C};

    cpu_test_reset(&cpu, NULL, code);
    cpu.h = 0xFF; /* 11111111 -> result=01111111, C=1 */
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x7F, cpu.h);
    assert_flags(&cpu, 0, 0, 0, 1);
}

/* === [HL] memory variants (r8=6) === */
void test_cb_rlc_hl(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xCB, 0x06};
    uint8_t ram[1] = {0x85};

    cpu_test_reset(&cpu, &mem, code);
    cpu.hl = (uint16_t)(uintptr_t)ram;
    cpu.f = 0;
    /* Override the code pointer: code needs to be at some address, but [HL] uses mem */
    /* Set up memory via mmu hooks if needed — for now, point HL at a byte in code space */
    /* Use a simpler approach: directly test via memory */
    write_memory8(&mem, 0xC000, 0x85);
    cpu.hl = 0xC000;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x0B, read_memory8(&mem, 0xC000));
    assert_flags(&cpu, 0, 0, 0, 1);
}

void test_cb_srl_hl(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xCB, 0x3E};

    cpu_test_reset(&cpu, &mem, code);
    write_memory8(&mem, 0xC000, 0x01);
    cpu.hl = 0xC000;
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, read_memory8(&mem, 0xC000));
    assert_flags(&cpu, 1, 0, 0, 1);
}

/* === BIT b3, r8 — Test bit (Z = !bit, N=0, H=1, C unchanged) === */

/* BIT 0, B — bit 0 set */
void test_cb_bit_0_b_set(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x40}; /* BIT 0, B */

    cpu_test_reset(&cpu, NULL, code);
    cpu.b = 0x01; /* bit 0 = 1 */
    cpu.f = F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x01, cpu.b); /* value unchanged */
    assert_flags(&cpu, 0, 0, 1, 1); /* Z=0 (bit set), N=0, H=1, C unchanged */
}

/* BIT 0, B — bit 0 clear */
void test_cb_bit_0_b_clear(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x40}; /* BIT 0, B */

    cpu_test_reset(&cpu, NULL, code);
    cpu.b = 0xFE; /* bit 0 = 0 */
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xFE, cpu.b); /* value unchanged */
    assert_flags(&cpu, 1, 0, 1, 0); /* Z=1 (bit clear), N=0, H=1, C unchanged */
}

/* BIT 3, C */
void test_cb_bit_3_c_set(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x59}; /* BIT 3, C (0x59 = 01 011 001) */

    cpu_test_reset(&cpu, NULL, code);
    cpu.c = 0x08; /* bit 3 = 1 */
    cpu.f = 0;
    cpu_test_run(&cpu);

    assert_flags(&cpu, 0, 0, 1, 0);
}

void test_cb_bit_3_c_clear(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x59}; /* BIT 3, C */

    cpu_test_reset(&cpu, NULL, code);
    cpu.c = 0xF7; /* bit 3 = 0 */
    cpu.f = F_C;
    cpu_test_run(&cpu);

    assert_flags(&cpu, 1, 0, 1, 1);
}

/* BIT 7, A — most significant bit */
void test_cb_bit_7_a_set(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x7F}; /* BIT 7, A (0x7F = 01 111 111) */

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x80;
    cpu.f = 0;
    cpu_test_run(&cpu);

    assert_flags(&cpu, 0, 0, 1, 0);
}

void test_cb_bit_7_a_clear(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x7F}; /* BIT 7, A */

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x7F;
    cpu.f = F_Z | F_N | F_H | F_C;
    cpu_test_run(&cpu);

    assert_flags(&cpu, 1, 0, 1, 1); /* Z=1 (bit clear), N=0, H=1, C unchanged (1) */
}

/* BIT with [HL] (memory operand) */
void test_cb_bit_0_hl_set(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xCB, 0x46}; /* BIT 0, [HL] (0x46 = 01 000 110) */

    cpu_test_reset(&cpu, &mem, code);
    write_memory8(&mem, 0xC000, 0x01);
    cpu.hl = 0xC000;
    cpu.f = 0;
    cpu_test_run(&cpu);

    assert_flags(&cpu, 0, 0, 1, 0); /* Z=0 (bit set) */
}

void test_cb_bit_0_hl_clear(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xCB, 0x46}; /* BIT 0, [HL] */

    cpu_test_reset(&cpu, &mem, code);
    write_memory8(&mem, 0xC000, 0x00);
    cpu.hl = 0xC000;
    cpu.f = F_C;
    cpu_test_run(&cpu);

    assert_flags(&cpu, 1, 0, 1, 1); /* Z=1 (bit clear), C unchanged */
}

/* === RES b3, r8 — Reset (clear) bit (flags unchanged) === */

/* RES 0, B — clear bit 0 */
void test_cb_res_0_b(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x80}; /* RES 0, B (0x80 = 10 000 000) */

    cpu_test_reset(&cpu, NULL, code);
    cpu.b = 0xFF;
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xFE, cpu.b); /* bit 0 cleared */
    assert_flags_unchanged(&cpu, 0xFF);
}

/* RES 0, B — already clear */
void test_cb_res_0_b_already_clear(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x80}; /* RES 0, B */

    cpu_test_reset(&cpu, NULL, code);
    cpu.b = 0x00;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.b);
    assert_flags_unchanged(&cpu, 0);
}

/* RES 3, C */
void test_cb_res_3_c(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x99}; /* RES 3, C (0x99 = 10 011 001) */

    cpu_test_reset(&cpu, NULL, code);
    cpu.c = 0xFF;
    cpu.f = F_Z | F_N | F_H | F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xF7, cpu.c); /* bit 3 cleared */
    assert_flags_unchanged(&cpu, F_Z | F_N | F_H | F_C);
}

/* RES 7, A — clear msb */
void test_cb_res_7_a(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0xBF}; /* RES 7, A (0xBF = 10 111 111) */

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x80;
    cpu.f = F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.a);
    assert_flags_unchanged(&cpu, F_C);
}

/* RES with [HL] (memory) */
void test_cb_res_0_hl(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xCB, 0x86}; /* RES 0, [HL] (0x86 = 10 000 110) */

    cpu_test_reset(&cpu, &mem, code);
    write_memory8(&mem, 0xC000, 0xFF);
    cpu.hl = 0xC000;
    cpu.f = F_Z | F_N | F_H | F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xFE, read_memory8(&mem, 0xC000));
    assert_flags_unchanged(&cpu, F_Z | F_N | F_H | F_C);
}

/* === SET b3, r8 — Set bit (flags unchanged) === */

/* SET 0, B — set bit 0 */
void test_cb_set_0_b(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0xC0}; /* SET 0, B (0xC0 = 11 000 000) */

    cpu_test_reset(&cpu, NULL, code);
    cpu.b = 0x00;
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x01, cpu.b); /* bit 0 set */
    assert_flags_unchanged(&cpu, 0xFF);
}

/* SET 0, B — already set */
void test_cb_set_0_b_already_set(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0xC0}; /* SET 0, B */

    cpu_test_reset(&cpu, NULL, code);
    cpu.b = 0x01;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x01, cpu.b);
    assert_flags_unchanged(&cpu, 0);
}

/* SET 3, C */
void test_cb_set_3_c(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0xD9}; /* SET 3, C (0xD9 = 11 011 001) */

    cpu_test_reset(&cpu, NULL, code);
    cpu.c = 0x00;
    cpu.f = F_Z | F_N | F_H | F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x08, cpu.c); /* bit 3 set */
    assert_flags_unchanged(&cpu, F_Z | F_N | F_H | F_C);
}

/* SET 7, A — set msb */
void test_cb_set_7_a(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0xFF}; /* SET 7, A (0xFF = 11 111 111) */

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x00;
    cpu.f = F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x80, cpu.a);
    assert_flags_unchanged(&cpu, F_C);
}

/* SET 5, L */
void test_cb_set_5_l(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0xED}; /* SET 5, L (0xED = 11 101 101) */

    cpu_test_reset(&cpu, NULL, code);
    cpu.l = 0x00;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x20, cpu.l); /* bit 5 set */
    assert_flags_unchanged(&cpu, 0);
}

/* SET with [HL] (memory) */
void test_cb_set_0_hl(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xCB, 0xC6}; /* SET 0, [HL] (0xC6 = 11 000 110) */

    cpu_test_reset(&cpu, &mem, code);
    write_memory8(&mem, 0xC000, 0x00);
    cpu.hl = 0xC000;
    cpu.f = F_Z | F_N | F_H | F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x01, read_memory8(&mem, 0xC000));
    assert_flags_unchanged(&cpu, F_Z | F_N | F_H | F_C);
}

/* === BIT/RES/SET cycles and PC advance === */
void test_cb_bit_instruction_cycles(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x40}; /* BIT 0, B */

    cpu_test_reset(&cpu, NULL, code);
    cpu.b = 0x00;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(2, cpu.pc);
    TEST_ASSERT_EQUAL_UINT64(2, cpu.cycles);
}

void test_cb_res_instruction_cycles(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x80}; /* RES 0, B */

    cpu_test_reset(&cpu, NULL, code);
    cpu.b = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(2, cpu.pc);
    TEST_ASSERT_EQUAL_UINT64(2, cpu.cycles);
}

void test_cb_set_instruction_cycles(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0xC0}; /* SET 0, B */

    cpu_test_reset(&cpu, NULL, code);
    cpu.b = 0x00;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(2, cpu.pc);
    TEST_ASSERT_EQUAL_UINT64(2, cpu.cycles);
}

/* === Cycles and PC advance === */
void test_cb_instruction_cycles(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCB, 0x00};

    cpu_test_reset(&cpu, NULL, code);
    cpu.b = 0x00;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(2, cpu.pc);
    TEST_ASSERT_EQUAL_UINT64(2, cpu.cycles);
}