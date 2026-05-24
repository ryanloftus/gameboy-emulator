#include "cpu_test_util.h"

void test_noop(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0};

    cpu_test_reset(&cpu, NULL, code);
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0, cpu.af);
    TEST_ASSERT_EQUAL_UINT16(0, cpu.bc);
    TEST_ASSERT_EQUAL_UINT16(0, cpu.de);
    TEST_ASSERT_EQUAL_UINT16(0, cpu.hl);
    TEST_ASSERT_EQUAL_UINT16(0, cpu.sp);
    TEST_ASSERT_EQUAL_UINT16(1, cpu.pc);
}

void test_inc_r8(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0b00001100, 0b00000100};

    cpu_test_reset(&cpu, NULL, code);
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(1, cpu.c);
    TEST_ASSERT_EQUAL_UINT16(1, cpu.bc);
    TEST_ASSERT_EQUAL_UINT16(1, cpu.pc);

    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(1, cpu.c);
    TEST_ASSERT_EQUAL_UINT16((1 << 8) + 1, cpu.bc);
    TEST_ASSERT_EQUAL_UINT16(2, cpu.pc);
}

void test_dec_r8(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0b00001101, 0b00000101};

    cpu_test_reset(&cpu, NULL, code);
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xff, cpu.c);
    TEST_ASSERT_EQUAL_UINT16(0x00ff, cpu.bc);
    TEST_ASSERT_EQUAL_UINT16(1, cpu.pc);

    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xff, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xff, cpu.c);
    TEST_ASSERT_EQUAL_UINT16(0xffff, cpu.bc);
    TEST_ASSERT_EQUAL_UINT16(2, cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(1, cpu_test_flag_n(&cpu));
}

void test_inc_r16(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0b100011};

    cpu_test_reset(&cpu, NULL, code);
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(1, cpu.hl);
    TEST_ASSERT_EQUAL_UINT16(1, cpu.pc);
}

void test_dec_r16(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0b101011};

    cpu_test_reset(&cpu, NULL, code);
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0xFFFF, cpu.hl);
    TEST_ASSERT_EQUAL_UINT16(1, cpu.pc);
}

void test_add_hl_r16(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0b011001};

    cpu_test_reset(&cpu, NULL, code);
    cpu.de = 31;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(31, cpu.hl);
    TEST_ASSERT_EQUAL_UINT16(1, cpu.pc);
}
void test_zero_flag(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0b00001100, 0b00001101};

    cpu_test_reset(&cpu, NULL, code);
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0, cpu.f);
    TEST_ASSERT_EQUAL_UINT8(1, cpu.c);

    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(1, cpu_test_flag_z(&cpu));
}
