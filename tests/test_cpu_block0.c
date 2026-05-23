#include "cpu_test_util.h"

#include "mmu.h"

void test_ld_r16_imm16(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x01, 0x34, 0x12};

    cpu_test_reset(&cpu, NULL, code);
    cpu.f = 0xF0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0x1234, cpu.bc);
    TEST_ASSERT_EQUAL_UINT16(3, cpu.pc);
    assert_flags_unchanged(&cpu, 0xF0);
}

void test_inc_r16_all_pairs(void)
{
    const uint8_t opcodes[] = {0x03, 0x13, 0x23, 0x33};

    for (int i = 0; i < 4; ++i)
    {
        virtual_cpu cpu;
        uint8_t code[] = {opcodes[i]};

        cpu_test_reset(&cpu, NULL, code);
        cpu.f = 0xF0;
        if (i == 0)
        {
            cpu.bc = 0xFFFF;
        }
        else if (i == 1)
        {
            cpu.de = 0xFFFF;
        }
        else if (i == 2)
        {
            cpu.hl = 0xFFFF;
        }
        else
        {
            cpu.sp = 0xFFFF;
        }

        cpu_test_run(&cpu);

        if (i == 0)
        {
            TEST_ASSERT_EQUAL_UINT16(0x0000, cpu.bc);
        }
        else if (i == 1)
        {
            TEST_ASSERT_EQUAL_UINT16(0x0000, cpu.de);
        }
        else if (i == 2)
        {
            TEST_ASSERT_EQUAL_UINT16(0x0000, cpu.hl);
        }
        else
        {
            TEST_ASSERT_EQUAL_UINT16(0x0000, cpu.sp);
        }
        assert_flags_unchanged(&cpu, 0xF0);
        TEST_ASSERT_EQUAL_UINT16(1, cpu.pc);
    }
}

void test_add_hl_r16_preserves_z(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x09};

    cpu_test_reset(&cpu, NULL, code);
    cpu.hl = 0x0FFF;
    cpu.bc = 0x0001;
    cpu.f = F_Z | F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0x1000, cpu.hl);
    assert_flags(&cpu, 1, 0, 1, 0);
}

void test_inc_r8_half_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x0C};

    cpu_test_reset(&cpu, NULL, code);
    cpu.c = 0x0F;
    cpu.f = F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x10, cpu.c);
    assert_flags(&cpu, 0, 0, 1, 1);
}

void test_ld_r8_imm8(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x06, 0x42};

    cpu_test_reset(&cpu, NULL, code);
    cpu.f = 0xF0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x42, cpu.b);
    TEST_ASSERT_EQUAL_UINT16(2, cpu.pc);
    assert_flags_unchanged(&cpu, 0xF0);
}

void test_rlca(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x07};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x81;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x03, cpu.a);
    assert_flags(&cpu, 0, 0, 0, 1);
}

void test_rrca(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x0F};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x01;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x80, cpu.a);
    assert_flags(&cpu, 0, 0, 0, 1);
}

void test_rla(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x17};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x80;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.a);
    assert_flags(&cpu, 0, 0, 0, 1);
}

void test_rra(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x1F};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x01;
    cpu.f = F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x80, cpu.a);
    assert_flags(&cpu, 0, 0, 0, 1);
}

void test_cpl(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x2F};

    cpu_test_reset(&cpu, NULL, code);
    cpu.a = 0x3A;
    cpu.f = F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xC5, cpu.a);
    assert_flags(&cpu, 0, 1, 1, 1);
}

void test_scf(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x37};

    cpu_test_reset(&cpu, NULL, code);
    cpu.f = F_Z | F_N | F_H;
    cpu_test_run(&cpu);

    assert_flags(&cpu, 1, 0, 0, 1);
}

void test_ccf(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x3F, 0x3F};

    cpu_test_reset(&cpu, NULL, code);
    cpu.f = 0;
    cpu_test_run(&cpu);
    assert_flags(&cpu, 0, 0, 0, 1);

    cpu_test_run(&cpu);
    assert_flags(&cpu, 0, 0, 0, 0);
}

void test_inc_r8_hl_memory(void)
{
    memory mem;
    virtual_cpu cpu;
    uint8_t code[] = {0x34};

    cpu_test_reset(&cpu, &mem, code);
    cpu.hl = WORK_RAM_START;
    mem.raw[WORK_RAM_START] = 0x0F;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x10, mem.raw[WORK_RAM_START]);
}

void test_zero_flag_inc_dec(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0x0C, 0x0D};

    cpu_test_reset(&cpu, NULL, code);
    cpu_test_run(&cpu);
    TEST_ASSERT_EQUAL_UINT8(1, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0, cpu_test_flag_z(&cpu));

    cpu_test_run(&cpu);
    TEST_ASSERT_EQUAL_UINT8(0, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(1, cpu_test_flag_z(&cpu));
}
