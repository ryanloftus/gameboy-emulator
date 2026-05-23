#include "cpu_test_util.h"

#include "mmu.h"

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
    uint8_t code[] = {0x98};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x42;
    cpu.b = 0x43;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.a);
    assert_flags(&cpu, 0, 1, 1, 1);
}

void test_sbc_a_r8(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x99};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x10;
    cpu.c = 0x05;
    cpu.f = F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x0A, cpu.a);
    assert_flags(&cpu, 0, 1, 1, 0);
}

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
