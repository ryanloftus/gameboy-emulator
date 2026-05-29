#include "unity.h"
#include "cpu.h"
#include "mmu.h"

#include <string.h>

/* Helper: compute the index into io_registers array for a given 16-bit address */
#define IO_IDX(addr) ((addr) & 0xFF)

/* Test that DIV starts at 0 */
void test_div_starts_at_zero(void)
{
    memory mem;
    init_memory(&mem, NULL);

    uint8_t div = read_memory8(&mem, DIV_REG_ADDR);
    TEST_ASSERT_EQUAL_UINT8(0, div);
}

/* Test that DIV increments every 256 T-cycles (upper 8 bits of div_counter) */
void test_div_increments_every_256_cycles(void)
{
    memory mem;
    init_memory(&mem, NULL);

    /* After 256 cycles, DIV should become 1 */
    mem.div_counter += 255;
    TEST_ASSERT_EQUAL_UINT8(0, read_memory8(&mem, DIV_REG_ADDR));

    mem.div_counter += 1;  /* now at 256 total => 256/256 = 1 */
    TEST_ASSERT_EQUAL_UINT8(1, read_memory8(&mem, DIV_REG_ADDR));

    /* After another 256, should be 2 */
    mem.div_counter += 256;  /* now 512 */
    TEST_ASSERT_EQUAL_UINT8(2, read_memory8(&mem, DIV_REG_ADDR));

    /* After 254 more, should be 2 still (766/256 = 2) */
    mem.div_counter += 254;  /* now 766 */
    TEST_ASSERT_EQUAL_UINT8(2, read_memory8(&mem, DIV_REG_ADDR));

    /* After 2 more, should roll to 3 (768/256 = 3) */
    mem.div_counter += 2;    /* now 768 */
    TEST_ASSERT_EQUAL_UINT8(3, read_memory8(&mem, DIV_REG_ADDR));

    /* After another 256, should be 4 */
    mem.div_counter += 256;  /* now 1024 */
    TEST_ASSERT_EQUAL_UINT8(4, read_memory8(&mem, DIV_REG_ADDR));
}

/* Test that writing to DIV resets the internal divider counter to 0 */
void test_div_write_resets_divider(void)
{
    memory mem;
    init_memory(&mem, NULL);

    /* Advance the divider past 0 */
    mem.div_counter = 0x1234;
    TEST_ASSERT_NOT_EQUAL(0, read_memory8(&mem, DIV_REG_ADDR));

    /* Write any value to DIV to reset */
    write_memory8(&mem, DIV_REG_ADDR, 0xAB);
    TEST_ASSERT_EQUAL_UINT8(0, read_memory8(&mem, DIV_REG_ADDR));
    TEST_ASSERT_EQUAL_UINT16(0, mem.div_counter);
}

/* Test that update_timers increments div_counter */
void test_update_timers_increments_div_counter(void)
{
    memory mem;
    virtual_cpu cpu;
    init_memory(&mem, NULL);
    create_virtual_cpu(&cpu, &mem);

    TEST_ASSERT_EQUAL_UINT16(0, mem.div_counter);

    update_timers(&cpu, 4);
    TEST_ASSERT_EQUAL_UINT16(4, mem.div_counter);

    update_timers(&cpu, 100);
    TEST_ASSERT_EQUAL_UINT16(104, mem.div_counter);
}

/* Test that TAC bit 2 = 0 means timer disabled (TIMA doesn't increment) */
void test_tima_does_not_increment_when_disabled(void)
{
    memory mem;
    virtual_cpu cpu;
    init_memory(&mem, NULL);
    create_virtual_cpu(&cpu, &mem);

    /* Ensure TAC is 0 (timer disabled) */
    mem.io_registers[IO_IDX(TAC_REG_ADDR)] = 0x00;
    mem.io_registers[IO_IDX(TIMA_REG_ADDR)] = 0x00;

    /* Run many cycles — TIMA should stay at 0 */
    update_timers(&cpu, 10000);
    TEST_ASSERT_EQUAL_UINT8(0, mem.io_registers[IO_IDX(TIMA_REG_ADDR)]);
}

/* Test TAC bit 2 = 1 enables TIMA at 4096 Hz (threshold 1024) */
void test_tima_increments_at_4096hz(void)
{
    memory mem;
    virtual_cpu cpu;
    init_memory(&mem, NULL);
    create_virtual_cpu(&cpu, &mem);

    /* TAC: enable=1, clock=00 => 4096 Hz, threshold = 1024 */
    mem.io_registers[IO_IDX(TAC_REG_ADDR)] = 0x04;   /* bit 2 set, bits 1-0 = 00 */
    mem.io_registers[IO_IDX(TIMA_REG_ADDR)] = 0x00;
    mem.tima_accum = 0;

    /* 1023 cycles: no increment */
    update_timers(&cpu, 1023);
    TEST_ASSERT_EQUAL_UINT8(0, mem.io_registers[IO_IDX(TIMA_REG_ADDR)]);

    /* 1 more cycle (total 1024): increments to 1 */
    update_timers(&cpu, 1);
    TEST_ASSERT_EQUAL_UINT8(1, mem.io_registers[IO_IDX(TIMA_REG_ADDR)]);

    /* Another 1024: increments to 2 */
    update_timers(&cpu, 1024);
    TEST_ASSERT_EQUAL_UINT8(2, mem.io_registers[IO_IDX(TIMA_REG_ADDR)]);
}

/* Test TIMA at 262144 Hz (threshold 16) */
void test_tima_increments_at_262144hz(void)
{
    memory mem;
    virtual_cpu cpu;
    init_memory(&mem, NULL);
    create_virtual_cpu(&cpu, &mem);

    /* TAC: enable=1, clock=01 => 262144 Hz, threshold = 16 */
    mem.io_registers[IO_IDX(TAC_REG_ADDR)] = 0x05;   /* bit 2 set, bits 1-0 = 01 */
    mem.io_registers[IO_IDX(TIMA_REG_ADDR)] = 0x00;
    mem.tima_accum = 0;

    /* 15 cycles: no increment */
    update_timers(&cpu, 15);
    TEST_ASSERT_EQUAL_UINT8(0, mem.io_registers[IO_IDX(TIMA_REG_ADDR)]);

    /* 1 more: increments */
    update_timers(&cpu, 1);
    TEST_ASSERT_EQUAL_UINT8(1, mem.io_registers[IO_IDX(TIMA_REG_ADDR)]);
}

/* Test TIMA at 65536 Hz (threshold 64) */
void test_tima_increments_at_65536hz(void)
{
    memory mem;
    virtual_cpu cpu;
    init_memory(&mem, NULL);
    create_virtual_cpu(&cpu, &mem);

    /* TAC: enable=1, clock=10 => 65536 Hz, threshold = 64 */
    mem.io_registers[IO_IDX(TAC_REG_ADDR)] = 0x06;
    mem.io_registers[IO_IDX(TIMA_REG_ADDR)] = 0x00;
    mem.tima_accum = 0;

    update_timers(&cpu, 63);
    TEST_ASSERT_EQUAL_UINT8(0, mem.io_registers[IO_IDX(TIMA_REG_ADDR)]);

    update_timers(&cpu, 1);
    TEST_ASSERT_EQUAL_UINT8(1, mem.io_registers[IO_IDX(TIMA_REG_ADDR)]);
}

/* Test TIMA at 16384 Hz (threshold 256) */
void test_tima_increments_at_16384hz(void)
{
    memory mem;
    virtual_cpu cpu;
    init_memory(&mem, NULL);
    create_virtual_cpu(&cpu, &mem);

    /* TAC: enable=1, clock=11 => 16384 Hz, threshold = 256 */
    mem.io_registers[IO_IDX(TAC_REG_ADDR)] = 0x07;
    mem.io_registers[IO_IDX(TIMA_REG_ADDR)] = 0x00;
    mem.tima_accum = 0;

    update_timers(&cpu, 255);
    TEST_ASSERT_EQUAL_UINT8(0, mem.io_registers[IO_IDX(TIMA_REG_ADDR)]);

    update_timers(&cpu, 1);
    TEST_ASSERT_EQUAL_UINT8(1, mem.io_registers[IO_IDX(TIMA_REG_ADDR)]);
}

/* Test that TIMA overflow reloads from TMA and sets timer interrupt */
void test_tima_overflow_reloads_from_tma_and_triggers_interrupt(void)
{
    memory mem;
    virtual_cpu cpu;
    init_memory(&mem, NULL);
    create_virtual_cpu(&cpu, &mem);

    /* TAC: enable=1, clock=00 => 4096 Hz (threshold 1024) */
    mem.io_registers[IO_IDX(TAC_REG_ADDR)] = 0x04;
    /* Set TMA reload value */
    mem.io_registers[IO_IDX(TMA_REG_ADDR)] = 0xAB;
    /* Set TIMA to just before overflow */
    mem.io_registers[IO_IDX(TIMA_REG_ADDR)] = 0xFF;
    /* Clear IF timer flag */
    mem.io_registers[IO_IDX(IF_REG_ADDR)] = 0;
    mem.tima_accum = 0;

    /* 1024 cycles: TIMA rolls over from 0xFF to 0, reloads from TMA */
    update_timers(&cpu, 1024);

    /* TIMA should contain the reload value */
    TEST_ASSERT_EQUAL_UINT8(0xAB, mem.io_registers[IO_IDX(TIMA_REG_ADDR)]);

    /* Timer interrupt flag (bit 2) should be set */
    TEST_ASSERT_TRUE(mem.io_registers[IO_IDX(IF_REG_ADDR)] & 0x04);
}

/* Test that TIMA overflow preserves other IF bits */
void test_tima_overflow_preserves_other_if_bits(void)
{
    memory mem;
    virtual_cpu cpu;
    init_memory(&mem, NULL);
    create_virtual_cpu(&cpu, &mem);

    mem.io_registers[IO_IDX(TAC_REG_ADDR)] = 0x04;
    mem.io_registers[IO_IDX(TMA_REG_ADDR)] = 0x00;
    mem.io_registers[IO_IDX(TIMA_REG_ADDR)] = 0xFF;
    /* Set VBlank and LCDC interrupt flags (bits 0 and 1) */
    mem.io_registers[IO_IDX(IF_REG_ADDR)] = 0x03;
    mem.tima_accum = 0;

    update_timers(&cpu, 1024);

    /* Timer bit (2) set, existing bits 0 and 1 preserved */
    TEST_ASSERT_EQUAL_UINT8(0x07, mem.io_registers[IO_IDX(IF_REG_ADDR)]);
}

/* Test TAC clock select bits independently of enable bit */
void test_tac_clock_select_works_with_enable(void)
{
    memory mem;
    virtual_cpu cpu;
    init_memory(&mem, NULL);
    create_virtual_cpu(&cpu, &mem);

    /* Test each clock setting individually */
    uint8_t clocks[] = {0x04, 0x05, 0x06, 0x07};
    uint16_t thresholds[] = {1024, 16, 64, 256};

    for (int i = 0; i < 4; i++) {
        mem.io_registers[IO_IDX(TAC_REG_ADDR)] = clocks[i];
        mem.io_registers[IO_IDX(TIMA_REG_ADDR)] = 0x00;
        mem.tima_accum = 0;

        /* Just under threshold — no increment */
        update_timers(&cpu, thresholds[i] - 1);
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x00, mem.io_registers[IO_IDX(TIMA_REG_ADDR)],
            "TIMA should not increment before threshold");

        /* Exactly threshold — should increment */
        update_timers(&cpu, 1);
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x01, mem.io_registers[IO_IDX(TIMA_REG_ADDR)],
            "TIMA should increment at threshold");
    }
}

/* Test that update_timers can handle multiple TIMA increments in a single call
   (e.g., if cycles_elapsed > threshold, multiple increments occur) */
void test_multiple_tima_increments_in_one_call(void)
{
    memory mem;
    virtual_cpu cpu;
    init_memory(&mem, NULL);
    create_virtual_cpu(&cpu, &mem);

    /* Fastest clock: threshold = 16 */
    mem.io_registers[IO_IDX(TAC_REG_ADDR)] = 0x05;   /* enable=1, clock=01 */
    mem.io_registers[IO_IDX(TIMA_REG_ADDR)] = 0x00;
    mem.tima_accum = 0;

    /* 48 cycles = 3 increments */
    update_timers(&cpu, 48);
    TEST_ASSERT_EQUAL_UINT8(3, mem.io_registers[IO_IDX(TIMA_REG_ADDR)]);
}

/* Test that TIMA overflow with multiple increments in one call works correctly */
void test_multiple_overflow_in_one_call(void)
{
    memory mem;
    virtual_cpu cpu;
    init_memory(&mem, NULL);
    create_virtual_cpu(&cpu, &mem);

    /* Fastest clock: threshold = 16 */
    mem.io_registers[IO_IDX(TAC_REG_ADDR)] = 0x05;   /* enable=1, clock=01 */
    mem.io_registers[IO_IDX(TMA_REG_ADDR)] = 0x42;
    mem.io_registers[IO_IDX(TIMA_REG_ADDR)] = 0xFE;  /* 2 away from overflow */
    mem.io_registers[IO_IDX(IF_REG_ADDR)] = 0;
    mem.tima_accum = 0;

    /* 48 cycles = 3 increments: FE -> FF -> overflow! (reload to 0x42, set IF) -> 0x43 */
    update_timers(&cpu, 48);

    /* After overflow: TIMA = TMA + 1 = 0x42 + 1 = 0x43 */
    TEST_ASSERT_EQUAL_UINT8(0x43, mem.io_registers[IO_IDX(TIMA_REG_ADDR)]);
    /* Interrupt should be set */
    TEST_ASSERT_TRUE(mem.io_registers[IO_IDX(IF_REG_ADDR)] & 0x04);
}

/* Test that DIV rollover doesn't affect TIMA */
void test_div_rollover_independent_of_tima(void)
{
    memory mem;
    virtual_cpu cpu;
    init_memory(&mem, NULL);
    create_virtual_cpu(&cpu, &mem);

    /* Disable TIMA */
    mem.io_registers[IO_IDX(TAC_REG_ADDR)] = 0x00;
    mem.io_registers[IO_IDX(TIMA_REG_ADDR)] = 0x00;

    /* Overflow div_counter */
    mem.div_counter = 0xFFFE;
    update_timers(&cpu, 3);

    /* DIV should have wrapped */
    TEST_ASSERT_EQUAL_UINT8(0x00, read_memory8(&mem, DIV_REG_ADDR));
    /* TIMA should still be 0 (disabled) */
    TEST_ASSERT_EQUAL_UINT8(0x00, mem.io_registers[IO_IDX(TIMA_REG_ADDR)]);
}

/* Test TIMA value persists across disable/enable toggles */
void test_tima_persists_across_toggle(void)
{
    memory mem;
    virtual_cpu cpu;
    init_memory(&mem, NULL);
    create_virtual_cpu(&cpu, &mem);

    /* Enable timer, increment once */
    mem.io_registers[IO_IDX(TAC_REG_ADDR)] = 0x05;   /* enable=1, threshold=16 */
    mem.io_registers[IO_IDX(TIMA_REG_ADDR)] = 0x00;
    mem.tima_accum = 0;

    update_timers(&cpu, 16);
    TEST_ASSERT_EQUAL_UINT8(1, mem.io_registers[IO_IDX(TIMA_REG_ADDR)]);

    /* Disable timer, run many cycles */
    mem.io_registers[IO_IDX(TAC_REG_ADDR)] = 0x01;   /* enable=0, threshold preserved */
    update_timers(&cpu, 10000);
    TEST_ASSERT_EQUAL_UINT8(1, mem.io_registers[IO_IDX(TIMA_REG_ADDR)]);

    /* Re-enable, should continue from where it left off */
    mem.io_registers[IO_IDX(TAC_REG_ADDR)] = 0x05;
    update_timers(&cpu, 16);
    TEST_ASSERT_EQUAL_UINT8(2, mem.io_registers[IO_IDX(TIMA_REG_ADDR)]);
}

/* Test DIV can overflow the full 16-bit counter */
void test_div_counter_16bit_wraparound(void)
{
    memory mem;
    virtual_cpu cpu;
    init_memory(&mem, NULL);
    create_virtual_cpu(&cpu, &mem);

    mem.div_counter = 0xFF00;

    /* Read DIV before wrap: should be 0xFF */
    TEST_ASSERT_EQUAL_UINT8(0xFF, read_memory8(&mem, DIV_REG_ADDR));

    update_timers(&cpu, 256);

    /* Read DIV after wrap: should be 0x00 */
    TEST_ASSERT_EQUAL_UINT8(0x00, read_memory8(&mem, DIV_REG_ADDR));

    /* Internal counter should have wrapped to 0x0000 */
    TEST_ASSERT_EQUAL_UINT16(0x0000, mem.div_counter);
}