#include "cpu_test_util.h"

#include "mmu.h"

#include <stdint.h>

/* ======================== PUSH / POP ======================== */

void test_push_bc(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xC5};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    cpu.bc = 0x1234;
    cpu.sp = 0xFFFE;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0xFFFC, cpu.sp);
    TEST_ASSERT_EQUAL_UINT16(0x1234, read_memory16(&mem, 0xFFFC));
    TEST_ASSERT_EQUAL_UINT16(1, cpu.pc);
}

void test_push_de(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xD5};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    cpu.de = 0xABCD;
    cpu.sp = 0xFFF0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0xFFEE, cpu.sp);
    TEST_ASSERT_EQUAL_UINT16(0xABCD, read_memory16(&mem, 0xFFEE));
}

void test_push_hl(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xE5};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    cpu.hl = 0x5678;
    cpu.sp = 0xFFFE;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0xFFFC, cpu.sp);
    TEST_ASSERT_EQUAL_UINT16(0x5678, read_memory16(&mem, 0xFFFC));
}

void test_push_af(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xF5};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    cpu.af = 0x9AB0;
    cpu.sp = 0xFFFE;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0xFFFC, cpu.sp);
    TEST_ASSERT_EQUAL_UINT16(0x9AB0, read_memory16(&mem, 0xFFFC));
}

void test_pop_bc(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xC1};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    write_memory16(&mem, 0xFFFE, 0xABCD);
    cpu.sp = 0xFFFE;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0xABCD, cpu.bc);
    TEST_ASSERT_EQUAL_UINT16(0x0000 | (0xFFFE + 2), cpu.sp);
}

void test_pop_af_clears_lower_nibble(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xF1};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    write_memory16(&mem, 0xFFFE, 0xFFFE); /* F would be 0xFE if not masked */
    cpu.sp = 0xFFFE;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.a);
    TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.f); /* lower nibble cleared */
}

void test_pop_af(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xF1};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    write_memory16(&mem, 0xFFFC, 0x9A50);
    cpu.sp = 0xFFFC;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x9A, cpu.a);
    TEST_ASSERT_EQUAL_UINT8(0x50, cpu.f);
}

/* ======================== RET ======================== */

void test_ret(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xC9};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    write_memory16(&mem, 0xFFFE, 0x1234);
    cpu.sp = 0xFFFE;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0x1234, cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0x0000 | (0xFFFE + 2), cpu.sp);
}

void test_ret_nz_taken(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xC0};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    write_memory16(&mem, 0xFFFE, 0x5678);
    cpu.sp = 0xFFFE;
    cpu.f = 0; /* Z not set */
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0x5678, cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0x0000 | (0xFFFE + 2), cpu.sp);
}

void test_ret_nz_not_taken(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xC0};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    write_memory16(&mem, 0xFFFE, 0x5678);
    cpu.sp = 0xFFFE;
    cpu.f = F_Z; /* Z set -> not taken */
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(1, cpu.pc); /* PC advances normally */
    TEST_ASSERT_EQUAL_UINT16(0xFFFE, cpu.sp); /* SP unchanged */
}

void test_ret_z_taken(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xC8};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    write_memory16(&mem, 0xFFFC, 0x9ABC);
    cpu.sp = 0xFFFC;
    cpu.f = F_Z;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0x9ABC, cpu.pc);
}

void test_ret_c_taken(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xD8};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    write_memory16(&mem, 0xFFFE, 0x4242);
    cpu.sp = 0xFFFE;
    cpu.f = F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0x4242, cpu.pc);
}

void test_ret_nc_not_taken(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xD0};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    cpu.sp = 0xFFFE;
    cpu.f = F_C; /* C set -> NC not taken */
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(1, cpu.pc);
}

/* ======================== JP ======================== */

void test_jp(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xC3, 0x34, 0x12};

    cpu_test_reset(&cpu, NULL, code);
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0x1234, cpu.pc);
}

void test_jp_nz_taken(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xC2, 0x78, 0x56};

    cpu_test_reset(&cpu, NULL, code);
    cpu.f = 0; /* Z not set */
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0x5678, cpu.pc);
}

void test_jp_nz_not_taken(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xC2, 0x78, 0x56};

    cpu_test_reset(&cpu, NULL, code);
    cpu.f = F_Z; /* Z set -> not taken */
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(3, cpu.pc); /* 3-byte instruction */
}

void test_jp_z_taken(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xCA, 0x00, 0x80};

    cpu_test_reset(&cpu, NULL, code);
    cpu.f = F_Z;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0x8000, cpu.pc);
}

void test_jp_nc_taken(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xD2, 0x34, 0x12};

    cpu_test_reset(&cpu, NULL, code);
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0x1234, cpu.pc);
}

void test_jp_c_taken(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xDA, 0x34, 0x12};

    cpu_test_reset(&cpu, NULL, code);
    cpu.f = F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0x1234, cpu.pc);
}

void test_jp_hl(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xE9};

    cpu_test_reset(&cpu, NULL, code);
    cpu.hl = 0xABCD;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0xABCD, cpu.pc);
}

/* ======================== CALL ======================== */

void test_call(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xCD, 0x34, 0x12};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    cpu.sp = 0xFFFE;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0x1234, cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0xFFFC, cpu.sp);
    TEST_ASSERT_EQUAL_UINT16(3, read_memory16(&mem, 0xFFFC)); /* return address */
}

void test_call_nz_taken(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xC4, 0x34, 0x12};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    cpu.sp = 0xFFFE;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0x1234, cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0xFFFC, cpu.sp);
}

void test_call_nz_not_taken(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xC4, 0x34, 0x12};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    cpu.sp = 0xFFFE;
    cpu.f = F_Z;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(3, cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0xFFFE, cpu.sp); /* SP unchanged */
}

void test_call_z_taken(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xCC, 0x78, 0x56};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    cpu.sp = 0xFFFE;
    cpu.f = F_Z;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0x5678, cpu.pc);
}

void test_call_c_taken(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xDC, 0x00, 0x40};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    cpu.sp = 0xFFFE;
    cpu.f = F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0x4000, cpu.pc);
}

void test_call_nc_not_taken(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xD4, 0x34, 0x12};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    cpu.sp = 0xFFFE;
    cpu.f = F_C;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(3, cpu.pc);
}

/* ======================== RST ======================== */

void test_rst_00(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xC7};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    cpu.sp = 0xFFFE;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0x0000, cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0xFFFC, cpu.sp);
    TEST_ASSERT_EQUAL_UINT16(1, read_memory16(&mem, 0xFFFC));
}

void test_rst_38(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xFF};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    cpu.sp = 0xFFFE;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0x0038, cpu.pc);
}

void test_rst_10(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xD7};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    cpu.sp = 0xFFFE;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0x0010, cpu.pc);
}

/* ======================== LDH / LD ======================== */

void test_ldh_c_a(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xE2};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    cpu.a = 0x42;
    cpu.c = 0x80;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x42, read_memory8(&mem, 0xFF80));
}

void test_ldh_imm8_a(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xE0, 0x90};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    cpu.a = 0xAA;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xAA, read_memory8(&mem, 0xFF90));
}

void test_ld_imm16_a(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xEA, 0x00, 0xC0};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    cpu.a = 0x55;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x55, read_memory8(&mem, 0xC000));
}

void test_ldh_a_c(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xF2};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    write_memory8(&mem, 0xFF80, 0x77);
    cpu.c = 0x80;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0x77, cpu.a);
}

void test_ldh_a_imm8(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xF0, 0x90};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    write_memory8(&mem, 0xFF90, 0xBB);
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xBB, cpu.a);
}

void test_ld_a_imm16(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xFA, 0x00, 0xC0};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    write_memory8(&mem, 0xC000, 0xCC);
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0xCC, cpu.a);
}

/* ======================== SP/HL arithmetic ======================== */

void test_add_sp_imm8_positive(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xE8, 0x05};

    cpu_test_reset(&cpu, NULL, code);
    cpu.sp = 0xFFF8;
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0xFFFD, cpu.sp);
    assert_flags(&cpu, 0, 0, 0, 0);
}

void test_add_sp_imm8_negative(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xE8, 0xF0}; /* -16 */

    cpu_test_reset(&cpu, NULL, code);
    cpu.sp = 0x1000;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0x0FF0, cpu.sp);
}

void test_add_sp_imm8_half_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xE8, 0x01};

    cpu_test_reset(&cpu, NULL, code);
    cpu.sp = 0x000F;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0x0010, cpu.sp);
    assert_flags(&cpu, 0, 0, 1, 0);
}

void test_add_sp_imm8_carry(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xE8, 0x01};

    cpu_test_reset(&cpu, NULL, code);
    cpu.sp = 0x00FF;
    cpu.f = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0x0100, cpu.sp);
    assert_flags(&cpu, 0, 0, 1, 1);
}

void test_ld_hl_sp_plus_imm8(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xF8, 0x08};

    cpu_test_reset(&cpu, NULL, code);
    cpu.sp = 0xFFF0;
    cpu.f = 0xFF;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0xFFF8, cpu.hl);
    assert_flags(&cpu, 0, 0, 0, 0);
}

void test_ld_sp_hl(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xF9};

    cpu_test_reset(&cpu, NULL, code);
    cpu.hl = 0xABCD;
    cpu.sp = 0;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0xABCD, cpu.sp);
}

/* ======================== DI / EI ======================== */

void test_di_clears_ime(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xF3};

    cpu_test_reset(&cpu, &mem, code);
    write_memory8(&mem, 0xFFFF, 1); /* Set IME */
    cpu.ei_scheduled = 1;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(0, read_memory8(&mem, 0xFFFF));
    TEST_ASSERT_EQUAL_UINT8(0, cpu.ei_scheduled);
    TEST_ASSERT_EQUAL_UINT16(1, cpu.pc);
}

void test_ei_schedules_ime(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xFB, 0x00}; /* EI, NOP */

    cpu_test_reset(&cpu, &mem, code);
    write_memory8(&mem, 0xFFFF, 0); /* Clear IME */
    cpu_test_run(&cpu); /* EI — schedules but does not set ime yet */

    TEST_ASSERT_EQUAL_UINT8(1, cpu.ei_scheduled);
    TEST_ASSERT_EQUAL_UINT8(0, read_memory8(&mem, 0xFFFF)); /* not yet enabled */
    TEST_ASSERT_EQUAL_UINT16(1, cpu.pc);

    cpu_test_run(&cpu); /* NOP — ime should be applied before this executes */

    TEST_ASSERT_EQUAL_UINT8(0, cpu.ei_scheduled);
    TEST_ASSERT_EQUAL_UINT8(1, read_memory8(&mem, 0xFFFF)); /* now enabled */
    TEST_ASSERT_EQUAL_UINT16(2, cpu.pc);
}

/* ======================== RETI ======================== */

void test_reti_returns_and_enables_ime(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xD9};

    cpu_test_reset(&cpu, &mem, code);
    /* Use a stack address that doesn't overlap with 0xFFFF (IME/IE register).
       Write the return address at separate byte locations to avoid
       overwriting 0xFFFF with the high byte. */
    write_memory8(&mem, 0xFFFC, 0xCD); /* low byte of return addr */
    write_memory8(&mem, 0xFFFD, 0xAB); /* high byte of return addr */
    cpu.sp = 0xFFFC;
    write_memory8(&mem, 0xFFFF, 0); /* Clear IME */
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0xABCD, cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0x0000 | (0xFFFC + 2), cpu.sp);
    TEST_ASSERT_EQUAL_UINT8(1, read_memory8(&mem, 0xFFFF));
    TEST_ASSERT_EQUAL_UINT8(0, cpu.ei_scheduled);
}

/* ======================== Cycles ======================== */

void test_jp_cycles(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xC3, 0x34, 0x12};

    cpu_test_reset(&cpu, NULL, code);
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT64(4, cpu.cycles);
}

void test_jp_hl_cycles(void)
{
    virtual_cpu cpu;
    uint8_t code[] = {0xE9};

    cpu_test_reset(&cpu, NULL, code);
    cpu.hl = 0x1234;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT64(1, cpu.cycles);
}

void test_call_cycles(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xCD, 0x00, 0x80};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    cpu.sp = 0xFFFE;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT64(6, cpu.cycles);
}

void test_ret_cycles(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xC9};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    write_memory16(&mem, 0xFFFE, 0x0000);
    cpu.sp = 0xFFFE;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT64(4, cpu.cycles);
}

void test_push_cycles(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xC5};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    cpu.sp = 0xFFFE;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT64(4, cpu.cycles);
}

void test_pop_cycles(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xC1};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    write_memory16(&mem, 0xFFFE, 0x0000);
    cpu.sp = 0xFFFE;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT64(3, cpu.cycles);
}

void test_rst_cycles(void)
{
    virtual_cpu cpu;
    memory mem;
    uint8_t code[] = {0xC7};

    cpu_test_reset(&cpu, &mem, code);
    init_memory(&mem, NULL);
    cpu.sp = 0xFFFE;
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT64(4, cpu.cycles);
}