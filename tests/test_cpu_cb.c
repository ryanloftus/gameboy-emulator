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