#include "cpu_test_util.h"

#include "mmu.h"

/* ADD A,r8 — Flags: Z=Set if result 0, N=0, H=Set if overflow from bit 3, C=Set if overflow from bit 7 */
void test_add_a_r8(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x80};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x3C;
    cpu.b = 0x02;
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x3E, cpu.a);
    assert_flags(&cpu, 0, 0, 0, 0);
}

void test_add_a_r8_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x80};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0xFF;
    cpu.b = 0x01;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.a);
    assert_flags(&cpu, 1, 0, 1, 1);
}

void test_add_a_r8_half_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x80};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x0F;
    cpu.b = 0x01;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x10, cpu.a);
    assert_flags(&cpu, 0, 0, 1, 0);
}

void test_add_a_r8_no_half_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x80};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x08;
    cpu.b = 0x07;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x0F, cpu.a);
    assert_flags(&cpu, 0, 0, 0, 0);
}

/* ADC A,r8 — Same flags as ADD A,r8 */
void test_adc_a_r8_with_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x88};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0xFF;
    cpu.c = 0x00;
    cpu.f = F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.a);
    assert_flags(&cpu, 1, 0, 1, 1);
}

void test_adc_a_r8_without_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x89};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x10;
    cpu.c = 0x05;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x15, cpu.a);
    assert_flags(&cpu, 0, 0, 0, 0);
}

void test_adc_a_r8_half_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x88};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x0F;
    cpu.c = 0x00;
    cpu.f = F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x10, cpu.a);
    assert_flags(&cpu, 0, 0, 1, 0);
}

/* SUB A,r8 — Flags: Z=Set if result 0, N=1, H=Set if borrow from bit 4, C=Set if borrow */
void test_sub_a_r8(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x90};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x3E;
    cpu.b = 0x2A;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x14, cpu.a);
    assert_flags(&cpu, 0, 1, 0, 0);
}

void test_sub_a_r8_borrow(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x90};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x42;
    cpu.b = 0x43;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.a);
    assert_flags(&cpu, 0, 1, 1, 1);
}

void test_sub_a_r8_zero(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x90};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x42;
    cpu.b = 0x42;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.a);
    assert_flags(&cpu, 1, 1, 0, 0);
}

void test_sub_a_r8_half_borrow(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x90};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x10;
    cpu.b = 0x01;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x0F, cpu.a);
    assert_flags(&cpu, 0, 1, 1, 0);
}

/* SBC A,r8 — Opcode bits: alu_op=3(011), r8 field selects operand register */
void test_sbc_a_r8(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x98};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x10;
    cpu.b = 0x05;
    cpu.f = F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x0A, cpu.a);
    assert_flags(&cpu, 0, 1, 1, 0);
}

void test_sbc_a_r8_borrow_with_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x99};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x00;
    cpu.c = 0x01;
    cpu.f = F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xFE, cpu.a);
    assert_flags(&cpu, 0, 1, 1, 1);
}

void test_sbc_a_r8_zero(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x98};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x01;
    cpu.b = 0x01;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.a);
    assert_flags(&cpu, 1, 1, 0, 0);
}

/* AND A,r8 — Flags: Z=Set if result 0, N=0, H=1, C=0 */
void test_and_a_r8(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xA0};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0xCF;
    cpu.b = 0x0F;
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x0F, cpu.a);
    assert_flags(&cpu, 0, 0, 1, 0);
}

void test_and_a_r8_zero(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xA0};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0xF0;
    cpu.b = 0x0F;
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.a);
    assert_flags(&cpu, 1, 0, 1, 0);
}

/* XOR A,r8 — Flags: Z=Set if result 0, N=0, H=0, C=0 */
void test_xor_a_r8(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xA8};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0xFF;
    cpu.b = 0xFF;
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.a);
    assert_flags(&cpu, 1, 0, 0, 0);
}

void test_xor_a_r8_nonzero(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xA8};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0xFF;
    cpu.b = 0x0F;
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.a);
    assert_flags(&cpu, 0, 0, 0, 0);
}

/* OR A,r8 — Flags: Z=Set if result 0, N=0, H=0, C=0 */
void test_or_a_r8(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xB0};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x00;
    cpu.b = 0x0F;
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x0F, cpu.a);
    assert_flags(&cpu, 0, 0, 0, 0);
}

void test_or_a_r8_zero(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xB0};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x00;
    cpu.b = 0x00;
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.a);
    assert_flags(&cpu, 1, 0, 0, 0);
}

/* CP A,r8 — Same flags as SUB A,r8, but A unchanged */
void test_cp_a_r8(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xB8};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x42;
    cpu.b = 0x42;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x42, cpu.a);
    assert_flags(&cpu, 1, 1, 0, 0);
}

void test_cp_a_r8_borrow(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xB9};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x42;
    cpu.c = 0x43;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x42, cpu.a);
    assert_flags(&cpu, 0, 1, 1, 1);
}

void test_cp_a_r8_half_borrow(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xB8};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x10;
    cpu.b = 0x01;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x10, cpu.a);
    assert_flags(&cpu, 0, 1, 1, 0);
}

/* DAA — Decimal Adjust Accumulator */
void test_daa_after_add_no_adjustment(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x27};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x15;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x15, cpu.a);
    assert_flags(&cpu, 0, 0, 0, 0);
}

void test_daa_after_add_lower_nibble_overflow(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x27};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x0A;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x10, cpu.a);
    assert_flags(&cpu, 0, 0, 0, 0);
}

void test_daa_after_add_upper_nibble_overflow(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x27};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x9A;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.a);
    assert_flags(&cpu, 1, 0, 0, 1);
}

void test_daa_after_add_with_half_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x27};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x0A;
    cpu.f = F_H;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x10, cpu.a);
    assert_flags(&cpu, 0, 0, 0, 0);
}

void test_daa_after_sub(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x27};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x0F;
    cpu.f = F_N;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x0F, cpu.a);
    assert_flags(&cpu, 0, 1, 0, 0);
}

void test_daa_after_sub_with_half_borrow(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x27};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x0F;
    cpu.f = F_N | F_H;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x09, cpu.a);
    assert_flags(&cpu, 0, 1, 0, 0);
}

void test_daa_after_sub_with_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x27};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0xFF;
    cpu.f = F_N | F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x9F, cpu.a);
    assert_flags(&cpu, 0, 1, 0, 1);
}

void test_daa_after_sub_no_adjustment(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x27};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x15;
    cpu.f = F_N;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x15, cpu.a);
    assert_flags(&cpu, 0, 1, 0, 0);
}

/* ALU with [HL] memory operand */
void test_alu_hl_memory(void)
{
    memory mem;
    virtual_cpu cpu;
    uint8_t code[] = {0x86};

    cpu_test_reset(&cpu, &mem, code);
    cpu.hl = WORK_RAM_START;
    mem.raw[WORK_RAM_START] = 0x01;
    cpu.a = 0xFF;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.a);
    assert_flags(&cpu, 1, 0, 1, 1);
}