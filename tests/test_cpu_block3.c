#include "cpu_test_util.h"

#include "mmu.h"

/* ADD A,imm8 — Flags: Z=Set if result 0, N=0, H=Set if overflow from bit 3, C=Set if overflow from bit 7 */
void test_add_imm8(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xC6, 0x02};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x3C;
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x3E, cpu.a);
    assert_flags(&cpu, 0, 0, 0, 0);
}

void test_add_imm8_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xC6, 0x01};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0xFF;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.a);
    assert_flags(&cpu, 1, 0, 1, 1);
}

void test_add_imm8_half_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xC6, 0x01};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x0F;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x10, cpu.a);
    assert_flags(&cpu, 0, 0, 1, 0);
}

void test_add_imm8_no_half_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xC6, 0x07};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x08;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x0F, cpu.a);
    assert_flags(&cpu, 0, 0, 0, 0);
}

/* ADC A,imm8 — Same flags as ADD A,imm8 */
void test_adc_imm8_with_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCE, 0x00};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0xFF;
    cpu.f = F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.a);
    assert_flags(&cpu, 1, 0, 1, 1);
}

void test_adc_imm8_without_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCE, 0x05};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x10;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x15, cpu.a);
    assert_flags(&cpu, 0, 0, 0, 0);
}

void test_adc_imm8_half_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCE, 0x00};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x0F;
    cpu.f = F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x10, cpu.a);
    assert_flags(&cpu, 0, 0, 1, 0);
}

/* SUB A,imm8 — Flags: Z=Set if result 0, N=1, H=Set if borrow from bit 4, C=Set if borrow */
void test_sub_imm8(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xD6, 0x2A};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x3E;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x14, cpu.a);
    assert_flags(&cpu, 0, 1, 0, 0);
}

void test_sub_imm8_borrow(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xD6, 0x43};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x42;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.a);
    assert_flags(&cpu, 0, 1, 1, 1);
}

void test_sub_imm8_zero(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xD6, 0x42};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x42;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.a);
    assert_flags(&cpu, 1, 1, 0, 0);
}

void test_sub_imm8_half_borrow(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xD6, 0x01};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x10;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x0F, cpu.a);
    assert_flags(&cpu, 0, 1, 1, 0);
}

/* SBC A,imm8 */
void test_sbc_imm8(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xDE, 0x05};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x10;
    cpu.f = F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x0A, cpu.a);
    assert_flags(&cpu, 0, 1, 1, 0);
}

void test_sbc_imm8_borrow_with_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xDE, 0x01};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x00;
    cpu.f = F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xFE, cpu.a);
    assert_flags(&cpu, 0, 1, 1, 1);
}

void test_sbc_imm8_zero(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xDE, 0x01};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x01;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.a);
    assert_flags(&cpu, 1, 1, 0, 0);
}

/* AND A,imm8 — Flags: Z=Set if result 0, N=0, H=1, C=0 */
void test_and_imm8(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xE6, 0x0F};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0xCF;
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x0F, cpu.a);
    assert_flags(&cpu, 0, 0, 1, 0);
}

void test_and_imm8_zero(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xE6, 0x0F};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0xF0;
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.a);
    assert_flags(&cpu, 1, 0, 1, 0);
}

/* XOR A,imm8 — Flags: Z=Set if result 0, N=0, H=0, C=0 */
void test_xor_imm8(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xEE, 0xFF};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0xFF;
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.a);
    assert_flags(&cpu, 1, 0, 0, 0);
}

void test_xor_imm8_nonzero(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xEE, 0x0F};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0xFF;
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.a);
    assert_flags(&cpu, 0, 0, 0, 0);
}

/* OR A,imm8 — Flags: Z=Set if result 0, N=0, H=0, C=0 */
void test_or_imm8(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xF6, 0x0F};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x00;
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x0F, cpu.a);
    assert_flags(&cpu, 0, 0, 0, 0);
}

void test_or_imm8_zero(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xF6, 0x00};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x00;
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.a);
    assert_flags(&cpu, 1, 0, 0, 0);
}

/* CP A,imm8 — Same flags as SUB A,imm8, but A unchanged */
void test_cp_imm8(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xFE, 0x42};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x42;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x42, cpu.a);
    assert_flags(&cpu, 1, 1, 0, 0);
}

void test_cp_imm8_borrow(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xFE, 0x43};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x42;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x42, cpu.a);
    assert_flags(&cpu, 0, 1, 1, 1);
}

void test_cp_imm8_half_borrow(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xFE, 0x01};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x10;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x10, cpu.a);
    assert_flags(&cpu, 0, 1, 1, 0);
}