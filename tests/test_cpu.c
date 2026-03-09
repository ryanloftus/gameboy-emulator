#include "unity.h"
#include "cpu.h"

#include <stdint.h>

void setUp(void) {}
void tearDown(void) {}

void test_noop(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0};

    create_virtual_cpu(&cpu, NULL, code);

    fetch_execute(&cpu);

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

    create_virtual_cpu(&cpu, NULL, code);

    fetch_execute(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(1, cpu.c);
    TEST_ASSERT_EQUAL_UINT16(1, cpu.bc);
    TEST_ASSERT_EQUAL_UINT16(1, cpu.pc);

    fetch_execute(&cpu);

    TEST_ASSERT_EQUAL_UINT8(1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(1, cpu.c);
    TEST_ASSERT_EQUAL_UINT16((1 << 8) + 1, cpu.bc);
    TEST_ASSERT_EQUAL_UINT16(2, cpu.pc);
}

void test_dec_r8(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0b00001101, 0b00000101};

    create_virtual_cpu(&cpu, NULL, code);

    fetch_execute(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xff, cpu.c);
    TEST_ASSERT_EQUAL_UINT16(0x00ff, cpu.bc);
    TEST_ASSERT_EQUAL_UINT16(1, cpu.pc);

    fetch_execute(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xff, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xff, cpu.c);
    TEST_ASSERT_EQUAL_UINT16(0xffff, cpu.bc);
    TEST_ASSERT_EQUAL_UINT16(2, cpu.pc);

    uint8_t subtraction_flag = (cpu.f >> 6) & 1;
    TEST_ASSERT_EQUAL_UINT8(1, subtraction_flag);
}

void test_inc_r16(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0b100011};

    create_virtual_cpu(&cpu, NULL, code);

    fetch_execute(&cpu);

    TEST_ASSERT_EQUAL_UINT16(1, cpu.hl);
    TEST_ASSERT_EQUAL_UINT16(1, cpu.pc);
}

void test_dec_r16(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0b101011};

    create_virtual_cpu(&cpu, NULL, code);

    fetch_execute(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0xFFFF, cpu.hl);
    TEST_ASSERT_EQUAL_UINT16(1, cpu.pc);
}

void test_add_hl_r16(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0b011001};

    create_virtual_cpu(&cpu, NULL, code);
    cpu.de = 31;

    fetch_execute(&cpu);

    TEST_ASSERT_EQUAL_UINT16(31, cpu.hl);
    TEST_ASSERT_EQUAL_UINT16(1, cpu.pc);
}

void test_zero_flag(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0b00001100, 0b00001101};

    create_virtual_cpu(&cpu, NULL, code);

    fetch_execute(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0, cpu.f);
    TEST_ASSERT_EQUAL_UINT8(1, cpu.c);

    fetch_execute(&cpu);

    uint8_t zero_flag = cpu.f >> 7;
    TEST_ASSERT_EQUAL_UINT8(0, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(1, zero_flag);
}

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
    return UNITY_END();
}
