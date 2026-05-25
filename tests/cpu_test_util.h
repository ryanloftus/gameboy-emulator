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

static inline void cpu_test_reset(virtual_cpu *cpu, memory *mem, uint8_t *code)
{
    if (mem != NULL)
    {
        init_memory(mem, NULL);
    }
    create_virtual_cpu(cpu, mem, code);
}

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
