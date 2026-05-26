#ifndef CPU_TEST_UTIL_H
#define CPU_TEST_UTIL_H

#include "unity.h"
#include "cpu.h"
#include "mmu.h"

#include <stdint.h>
#include <string.h>

#define F_Z 0x80u
#define F_N 0x40u
#define F_H 0x20u
#define F_C 0x10u

/*
 * Internal fallback memory used when a test passes NULL for the memory
 * parameter.  Tests run sequentially so a single global is safe.
 */
static memory g_test_fallback_mem;

static inline void cpu_test_do_reset(virtual_cpu *cpu, memory *mem, const uint8_t *code, size_t code_len)
{
    memory *actual = mem ? mem : &g_test_fallback_mem;
    init_memory(actual, NULL);
    create_virtual_cpu(cpu, actual);

    /*
     * Wire the cartridge ROM pointer to the raw memory array and set
     * MBC type 0 (no mapper, linear access) so that is_cartridge_backed_addr
     * reads (0x0000–0x7FFF) return bytes directly from the array.
     * Tests that need real cartridge behaviour (e.g. MBC1 tests) override
     * this before calling cpu_test_run().
     */
    actual->cartridge.rom = actual->raw;
    actual->cartridge.rom_size = KB_64;
    actual->cartridge.mbc_type = 0;

    if (code != NULL && code_len > 0)
    {
        cpu->pc = 0;
        memcpy(actual->raw, code, code_len);
    }
}

/**
 * Macro: initialise a virtual CPU and write the test code into work RAM.
 *
 * @param cpu   pointer to a virtual_cpu
 * @param mem   pointer to a memory, or NULL to use a static fallback
 * @param code  a uint8_t array (its size is captured via sizeof at the
 *              macro call site, so it must be a local/global array, not
 *              a pointer)
 */
#define cpu_test_reset(cpu, mem, code)                              \
    cpu_test_do_reset((cpu), (mem), (code), sizeof(code))

static inline void cpu_test_run(virtual_cpu *cpu)
{
    fetch_execute(cpu);
}

static inline uint8_t cpu_test_flag_z(const virtual_cpu *cpu)
{
    return (cpu->f & F_Z) != 0;
}

static inline uint8_t cpu_test_flag_n(const virtual_cpu *cpu)
{
    return (cpu->f & F_N) != 0;
}

static inline uint8_t cpu_test_flag_h(const virtual_cpu *cpu)
{
    return (cpu->f & F_H) != 0;
}

static inline uint8_t cpu_test_flag_c(const virtual_cpu *cpu)
{
    return (cpu->f & F_C) != 0;
}

static inline void assert_flags(const virtual_cpu *cpu, uint8_t z, uint8_t n, uint8_t h, uint8_t c)
{
    TEST_ASSERT_EQUAL_UINT8(z, cpu_test_flag_z(cpu));
    TEST_ASSERT_EQUAL_UINT8(n, cpu_test_flag_n(cpu));
    TEST_ASSERT_EQUAL_UINT8(h, cpu_test_flag_h(cpu));
    TEST_ASSERT_EQUAL_UINT8(c, cpu_test_flag_c(cpu));
}

static inline void assert_flags_unchanged(const virtual_cpu *cpu, uint8_t f_before)
{
    TEST_ASSERT_EQUAL_UINT8(f_before, cpu->f);
}

#endif
