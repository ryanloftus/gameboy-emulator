#include "cpu_test_util.h"

#include "mmu.h"

void test_ld_r8_r8_bc(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x41};

    cpu_test_reset(&cpu, NULL, code);
    cpu.c = 0x55;
    cpu.b = 0;
    cpu.f = 0xF0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x55, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0x55, cpu.c);
    assert_flags_unchanged(&cpu, 0xF0);
    TEST_ASSERT_EQUAL_UINT16(1, cpu.pc);
}

void test_ld_r8_r8_self(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x40};

    cpu_test_reset(&cpu, NULL, code);
    cpu.b = 0x12;
    cpu.f = 0xF0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x12, cpu.b);
    assert_flags_unchanged(&cpu, 0xF0);
}

void test_ld_a_hl(void)
{
    memory mem;
    virtual_cpu cpu;
    uint8_t code[] = {0x7E};

    cpu_test_reset(&cpu, &mem, code);
    cpu.hl = WORK_RAM_START;
    mem.raw[WORK_RAM_START] = 0xAB;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xAB, cpu.a);
}

void test_ld_hl_a(void)
{
    memory mem;
    virtual_cpu cpu;
    uint8_t code[] = {0x77};

    cpu_test_reset(&cpu, &mem, code);
    cpu.hl = WORK_RAM_START;
    cpu.a = 0xCD;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xCD, mem.raw[WORK_RAM_START]);
}
