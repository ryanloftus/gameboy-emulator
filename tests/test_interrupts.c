#include "unity.h"
#include "cpu.h"
#include "mmu.h"
#include "cpu_test_util.h"

#include <stdint.h>
#include <string.h>

/* Helper: compute the index into io_registers array for a given 16-bit address */
#define IO_IDX(addr) ((addr) & 0xFF)

/* A single NOP instruction */
static const uint8_t nop_code[] = { 0x00 };

/* ===== Setup helpers ===== */

/**
 * Configure the timer such that the next call to update_timers with
 * NOP's cycle count (1) will cause a TIMA overflow.
 *
 * Uses the fastest clock (threshold=16) and pre-sets tima_accum so
 * that 1 more cycle causes an increment from 0xFF -> overflow.
 */
static void setup_timer_overflow_on_nop(virtual_cpu *cpu)
{
    memory *mem = cpu->mem;
    mem->io_registers[IO_IDX(TAC_REG_ADDR)] = 0x05;   /* enable=1, clock=01 => threshold=16 */
    mem->io_registers[IO_IDX(TIMA_REG_ADDR)] = 0xFF;  /* right before overflow */
    mem->io_registers[IO_IDX(TMA_REG_ADDR)] = 0x00;   /* reload value */
    mem->tima_accum = 15;  /* 15 + 1 (NOP cycles) = 16 = threshold */
    mem->div_counter = 0;
}

/**
 * Set IE bit 2 (timer enable) and IF bit 2 (timer request) simultaneously.
 */
static void request_timer_interrupt(memory *mem)
{
    mem->io_registers[IO_IDX(IF_REG_ADDR)] |= 0x04;  /* IF: request timer interrupt */
}

/* ===== Test: Timer interrupt fires when TIMA overflows with IME set ===== */

void test_timer_interrupt_fires_on_overflow(void)
{
    virtual_cpu cpu;
    memory mem;
    cpu_test_reset(&cpu, &mem, nop_code);

    /* Enable IME and enable timer in IE */
    write_memory8(&mem, 0xFFFF, 0xFF);  /* IE = all interrupts enabled */

    /* Set up timer to overflow on the NOP instruction */
    setup_timer_overflow_on_nop(&cpu);

    /* Execute one instruction.
     * fetch_execute sequence:
     *   1. service_interrupts() — checks IE & IF; IF not set yet → skip
     *   2. Execute NOP at pc=0 (4 cycles)
     *   3. update_timers(4) — tima_accum 12+4=16 → TIMA 0xFF → overflow → IF bit 2 set
     */
    fetch_execute(&cpu);

    /* After instruction: IF bit 2 should be set by update_timers,
     * but interrupt NOT yet serviced (service_interrupts ran before it).
     * PC should be 0x0001 (NOP executed). */
    TEST_ASSERT_EQUAL_UINT16(0x0001, cpu.pc);
    TEST_ASSERT_TRUE(mem.io_registers[IO_IDX(IF_REG_ADDR)] & 0x04);

    /* Next call: service_interrupts sees the pending timer interrupt */
    fetch_execute(&cpu);

    /* PC should be at timer vector 0x50 */
    TEST_ASSERT_EQUAL_UINT16(0x0050, cpu.pc);

    /* IME should be cleared */
    TEST_ASSERT_EQUAL_UINT8(0, read_memory8(&mem, 0xFFFF));

    /* IF bit 2 should be cleared */
    uint8_t iflag = mem.io_registers[IO_IDX(IF_REG_ADDR)];
    TEST_ASSERT_FALSE(iflag & 0x04);
}

/* ===== Test: Timer interrupt does NOT fire when IE bit 2 is clear ===== */

void test_timer_interrupt_does_not_fire_when_ie_bit2_clear(void)
{
    virtual_cpu cpu;
    memory mem;
    cpu_test_reset(&cpu, &mem, nop_code);

    /* IE = 1 (only VBlank enabled), so IME concept is "set" for VBlank only */
    mem.interrupt_enable_register = 1;
    /* Request timer interrupt via IF, but NOT enabled in IE */
    mem.io_registers[IO_IDX(IF_REG_ADDR)] = 0x04;

    fetch_execute(&cpu);

    /* PC should have advanced past NOP — interrupt NOT serviced */
    TEST_ASSERT_EQUAL_UINT16(0x0001, cpu.pc);

    /* IF bit 2 should remain set */
    TEST_ASSERT_TRUE(mem.io_registers[IO_IDX(IF_REG_ADDR)] & 0x04);
}

/* ===== Test: Timer interrupt does NOT fire when IE register is 0 ===== */

void test_timer_interrupt_does_not_fire_when_ie_is_zero(void)
{
    virtual_cpu cpu;
    memory mem;
    cpu_test_reset(&cpu, &mem, nop_code);

    /* IE = 0 */
    mem.interrupt_enable_register = 0;
    /* Request timer interrupt via IF */
    mem.io_registers[IO_IDX(IF_REG_ADDR)] |= 0x04;

    fetch_execute(&cpu);

    /* PC should have advanced past NOP — interrupt NOT serviced */
    TEST_ASSERT_EQUAL_UINT16(0x0001, cpu.pc);

    /* IF bit 2 should still be set */
    TEST_ASSERT_TRUE(mem.io_registers[IO_IDX(IF_REG_ADDR)] & 0x04);
}

/* ===== Test: Timer interrupt vectors to 0x50 and pushes PC ===== */

void test_timer_interrupt_vectors_to_0x50(void)
{
    virtual_cpu cpu;
    memory mem;
    cpu_test_reset(&cpu, &mem, nop_code);

    cpu.sp = 0xFFF0;
    mem.interrupt_enable_register = 0x04;  /* IE: timer enabled */
    request_timer_interrupt(&mem);

    fetch_execute(&cpu);

    /* PC jumps to timer vector */
    TEST_ASSERT_EQUAL_UINT16(0x0050, cpu.pc);

    /* Stack should contain the original PC (0x0000) */
    TEST_ASSERT_EQUAL_UINT16(0x0000, read_memory16(&mem, cpu.sp));
    TEST_ASSERT_EQUAL_UINT16(0xFFEE, cpu.sp);
}

/* ===== Test: IE is cleared after interrupt ===== */

void test_ie_cleared_after_interrupt(void)
{
    virtual_cpu cpu;
    memory mem;
    cpu_test_reset(&cpu, &mem, nop_code);

    mem.interrupt_enable_register = 0x04;  /* IE: timer enabled */
    request_timer_interrupt(&mem);

    fetch_execute(&cpu);

    /* IE register at 0xFFFF should be 0 (cleared by interrupt) */
    TEST_ASSERT_EQUAL_UINT8(0, read_memory8(&mem, 0xFFFF));
}

/* ===== Test: IF bit 2 is cleared, other IF bits preserved ===== */

void test_timer_interrupt_clears_if_bit2_preserves_others(void)
{
    virtual_cpu cpu;
    memory mem;
    cpu_test_reset(&cpu, &mem, nop_code);

    /* IE: only timer (bit 2) enabled */
    mem.interrupt_enable_register = 0x04;
    /* IF: VBlank (bit 0), LCDC (bit 1), and Timer (bit 2) */
    mem.io_registers[IO_IDX(IF_REG_ADDR)] = 0x07;

    fetch_execute(&cpu);

    /* Timer bit (2) should be cleared, VBlank (0) and LCDC (1) preserved */
    uint8_t iflag = mem.io_registers[IO_IDX(IF_REG_ADDR)];
    TEST_ASSERT_EQUAL_UINT8(0x03, iflag);
}

/* ===== Test: HALT is exited when timer interrupt fires ===== */

void test_halt_exited_by_timer_interrupt(void)
{
    virtual_cpu cpu;
    memory mem;
    cpu_test_reset(&cpu, &mem, nop_code);

    cpu.is_halted = 1;
    mem.interrupt_enable_register = 0x04;  /* IE: timer enabled */
    request_timer_interrupt(&mem);

    fetch_execute(&cpu);

    /* CPU should no longer be halted */
    TEST_ASSERT_EQUAL_UINT8(0, cpu.is_halted);

    /* Should have jumped to timer vector */
    TEST_ASSERT_EQUAL_UINT16(0x0050, cpu.pc);
}

/* ===== Test: HALT persists when no interrupt pending ===== */

void test_halt_persists_without_interrupt(void)
{
    virtual_cpu cpu;
    memory mem;
    cpu_test_reset(&cpu, &mem, nop_code);

    cpu.is_halted = 1;
    mem.interrupt_enable_register = 0x04;  /* IE: timer enabled */
    mem.io_registers[IO_IDX(IF_REG_ADDR)] = 0x00;  /* No interrupts pending */

    fetch_execute(&cpu);

    /* CPU should remain halted */
    TEST_ASSERT_EQUAL_UINT8(1, cpu.is_halted);

    /* PC should NOT have advanced */
    TEST_ASSERT_EQUAL_UINT16(0x0000, cpu.pc);
}

/* ===== Test: Higher priority interrupts fire before timer ===== */

void test_vblank_has_higher_priority_than_timer(void)
{
    virtual_cpu cpu;
    memory mem;
    cpu_test_reset(&cpu, &mem, nop_code);

    mem.interrupt_enable_register = 0xFF;  /* All interrupts enabled */
    /* Both VBlank (bit 0) and Timer (bit 2) pending */
    mem.io_registers[IO_IDX(IF_REG_ADDR)] = 0x05;  /* bits 0 and 2 */

    fetch_execute(&cpu);

    /* Should jump to VBlank vector (0x40), not timer (0x50) */
    TEST_ASSERT_EQUAL_UINT16(0x0040, cpu.pc);

    /* IF bit 0 should be cleared, bit 2 preserved */
    uint8_t iflag = mem.io_registers[IO_IDX(IF_REG_ADDR)];
    TEST_ASSERT_FALSE(iflag & 0x01);  /* bit 0 cleared */
    TEST_ASSERT_TRUE(iflag & 0x04);   /* bit 2 still set */
}

void test_lcdc_has_higher_priority_than_timer(void)
{
    virtual_cpu cpu;
    memory mem;
    cpu_test_reset(&cpu, &mem, nop_code);

    mem.interrupt_enable_register = 0xFF;  /* All interrupts enabled */
    /* Both LCDC (bit 1) and Timer (bit 2) pending */
    mem.io_registers[IO_IDX(IF_REG_ADDR)] = 0x06;  /* bits 1 and 2 */

    fetch_execute(&cpu);

    /* Should jump to LCDC vector (0x48) */
    TEST_ASSERT_EQUAL_UINT16(0x0048, cpu.pc);

    /* IF bit 1 should be cleared, bit 2 preserved */
    uint8_t iflag = mem.io_registers[IO_IDX(IF_REG_ADDR)];
    TEST_ASSERT_FALSE(iflag & 0x02);  /* bit 1 cleared */
    TEST_ASSERT_TRUE(iflag & 0x04);   /* bit 2 still set */
}

/* ===== Test: Timer overflow sets IF, then next cycle services it ===== */

void test_timer_overflow_sets_if_then_next_cycle_services_it(void)
{
    virtual_cpu cpu;
    memory mem;
    cpu_test_reset(&cpu, &mem, nop_code);

    mem.interrupt_enable_register = 0xFF;  /* All interrupts enabled */
    setup_timer_overflow_on_nop(&cpu);

    /* First cycle: NOP executes, TIMA overflows */
    fetch_execute(&cpu);
    TEST_ASSERT_EQUAL_UINT16(0x0001, cpu.pc);  /* NOP executed */
    TEST_ASSERT_TRUE(mem.io_registers[IO_IDX(IF_REG_ADDR)] & 0x04);  /* IF set by overflow */

    /* Second cycle: interrupt serviced */
    fetch_execute(&cpu);
    TEST_ASSERT_EQUAL_UINT16(0x0050, cpu.pc);  /* at timer vector */
    TEST_ASSERT_FALSE(mem.io_registers[IO_IDX(IF_REG_ADDR)] & 0x04);  /* IF cleared */
}

/* ===== Test: Timer overflow with non-zero TMA reload ===== */

void test_timer_overflow_with_tma_reload_triggers_interrupt(void)
{
    virtual_cpu cpu;
    memory mem;
    cpu_test_reset(&cpu, &mem, nop_code);

    mem.interrupt_enable_register = 0xFF;  /* All interrupts enabled */
    mem.io_registers[IO_IDX(TAC_REG_ADDR)] = 0x05;   /* enable=1, threshold=16 */
    mem.io_registers[IO_IDX(TIMA_REG_ADDR)] = 0xFF;
    mem.io_registers[IO_IDX(TMA_REG_ADDR)] = 0xAB;   /* TMA reload value */
    mem.tima_accum = 15;  /* 15 + 1 NOP cycle = 16 = threshold */
    mem.div_counter = 0;

    /* First cycle: overflow, TIMA reloads to 0xAB, IF set */
    fetch_execute(&cpu);
    TEST_ASSERT_EQUAL_UINT8(0xAB, mem.io_registers[IO_IDX(TIMA_REG_ADDR)]);
    TEST_ASSERT_TRUE(mem.io_registers[IO_IDX(IF_REG_ADDR)] & 0x04);
    TEST_ASSERT_EQUAL_UINT16(0x0001, cpu.pc);  /* NOP executed */

    /* Second cycle: interrupt serviced */
    fetch_execute(&cpu);
    TEST_ASSERT_EQUAL_UINT16(0x0050, cpu.pc);
}

/* ===== Test: Cycles are consumed by interrupt servicing ===== */

void test_timer_interrupt_consumes_cycles(void)
{
    virtual_cpu cpu;
    memory mem;
    cpu_test_reset(&cpu, &mem, nop_code);

    mem.interrupt_enable_register = 0x04;  /* IE: timer enabled */
    request_timer_interrupt(&mem);
    cpu.cycles = 0;

    fetch_execute(&cpu);

    /* Interrupt servicing takes 5 M-cycles */
    TEST_ASSERT_EQUAL_UINT64(5, cpu.cycles);
}

/* ===== Test: Push PC preserves correct address ===== */

void test_timer_interrupt_pushes_correct_pc(void)
{
    virtual_cpu cpu;
    memory mem;
    cpu_test_reset(&cpu, &mem, nop_code);

    /* Set PC to a non-zero value */
    cpu.pc = 0x1234;
    cpu.sp = 0xFFF0;
    mem.interrupt_enable_register = 0x04;  /* IE: timer enabled */
    request_timer_interrupt(&mem);

    fetch_execute(&cpu);

    /* PC was pushed before interrupt, so stack should have 0x1234 */
    TEST_ASSERT_EQUAL_UINT16(0x1234, read_memory16(&mem, cpu.sp));
}

/* ===== Test: Timer interrupt clears IF bit 2 only, keeps other IF bits ===== */

void test_timer_interrupt_clears_only_own_if_bit(void)
{
    virtual_cpu cpu;
    memory mem;
    cpu_test_reset(&cpu, &mem, nop_code);

    /* IE: timer + vblank enabled */
    mem.interrupt_enable_register = 0x05;
    /* IF: only timer bit set */
    mem.io_registers[IO_IDX(IF_REG_ADDR)] = 0x04;

    fetch_execute(&cpu);

    /* Should service timer interrupt (bit 2) */
    TEST_ASSERT_EQUAL_UINT16(0x0050, cpu.pc);
    /* IF bit 2 cleared, no other bits to preserve */
    TEST_ASSERT_EQUAL_UINT8(0x00, mem.io_registers[IO_IDX(IF_REG_ADDR)]);
}