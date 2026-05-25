#include "cpu_test_util.h"

/* ===== HALT Tests ===== */

void test_halt_sets_is_halted_ime_set(void)
{
    virtual_cpu cpu;
    /* HALT = 0x76, followed by NOP = 0x00 */
    uint8_t code[] = {0x76, 0x00};

    cpu_test_reset(&cpu, NULL, code);
    cpu.ime = 1;
    cpu_test_run(&cpu);

    /* With IME set, HALT should set is_halted */
    TEST_ASSERT_EQUAL_UINT8(1, cpu.is_halted);
    TEST_ASSERT_EQUAL_UINT16(1, cpu.pc); /* PC advanced past HALT */
}

void test_halt_sets_is_halted_ime_not_set_no_pending(void)
{
    virtual_cpu cpu;
    /* HALT = 0x76, followed by NOP = 0x00 */
    uint8_t code[] = {0x76, 0x00};

    cpu_test_reset(&cpu, NULL, code);
    cpu.ime = 0;
    /* No interrupts pending (IE & IF == 0) */
    cpu_test_run(&cpu);

    /* With IME not set and no pending interrupts, should set is_halted */
    TEST_ASSERT_EQUAL_UINT8(1, cpu.is_halted);
    TEST_ASSERT_EQUAL_UINT16(1, cpu.pc);
}

void test_halt_no_ime_pending_interrupt_pc_bug(void)
{
    /*
     * If IME is not set, and some interrupt is pending, the byte after HALT
     * is read twice (PC is not incremented properly - hardware bug)
     */
    virtual_cpu cpu;
    memory mem;

    uint8_t code[] = {0x76, 0x3C}; /* HALT then INC A (0x3C) */

    cpu_test_reset(&cpu, &mem, code);

    /* Set IE and IF AFTER cpu_test_reset, since it re-inits memory */
    write_memory8(&mem, 0xFFFF, 0x01); /* IE - bit 0 (VBlank) */
    write_memory8(&mem, 0xFF0F, 0x01); /* IF - bit 0 (VBlank) */

    cpu.ime = 0;
    /* IE & IF != 0, interrupt pending */
    cpu_test_run(&cpu);

    /* HALT bug: PC is NOT incremented - next byte is read twice */
    TEST_ASSERT_EQUAL_UINT16(0, cpu.pc);
    /* is_halted should NOT be set since we skip the halt */
    TEST_ASSERT_EQUAL_UINT8(0, cpu.is_halted);
}

/* ===== STOP Tests ===== */

void test_stop_sets_is_stopped(void)
{
    virtual_cpu cpu;
    /* STOP = 0x10, second byte can be anything (often 0x00) */
    uint8_t code[] = {0x10, 0x00};

    cpu_test_reset(&cpu, NULL, code);
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(1, cpu.is_stopped);
    TEST_ASSERT_EQUAL_UINT16(2, cpu.pc); /* PC advanced by 2 bytes */
    TEST_ASSERT_EQUAL_UINT64(0, cpu.cycles); /* 0 cycles */
}

void test_stop_ignores_second_byte(void)
{
    virtual_cpu cpu;
    /* Even with non-zero second byte, STOP should work */
    uint8_t code[] = {0x10, 0xFF};

    cpu_test_reset(&cpu, NULL, code);
    cpu_test_run(&cpu);

    TEST_ASSERT_EQUAL_UINT8(1, cpu.is_stopped);
    TEST_ASSERT_EQUAL_UINT16(2, cpu.pc);
}