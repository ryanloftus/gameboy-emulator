#include "cpu.h"
#include "testing_utils.h"

#include <stdio.h>
#include <memory.h>

int when_noop_then_inc_pc_only()
{
    virtual_cpu cpu;
    virtual_cpu prev_state;
    uint8_t code = 0;

    create_virtual_cpu(&cpu);
    memcpy(&prev_state, &cpu, sizeof(virtual_cpu));

    fetch_execute(&cpu, &code);

    assert(cpu.a == prev_state.a);
    assert(cpu.b == prev_state.b);
    assert(cpu.c == prev_state.c);
    assert(cpu.d == prev_state.d);
    assert(cpu.e == prev_state.e);
    assert(cpu.f == prev_state.f);
    assert(cpu.h == prev_state.h);
    assert(cpu.l == prev_state.l);
    assert(cpu.sp == prev_state.sp);
    assert(cpu.pc == prev_state.pc + 1);

    return PASSED;
}

int when_inc_b_then_b_incremented()
{
    virtual_cpu cpu;
    virtual_cpu prev_state;
    uint8_t code = 0b00000100;

    create_virtual_cpu(&cpu);
    memcpy(&prev_state, &cpu, sizeof(virtual_cpu));

    fetch_execute(&cpu, &code);

    assert(cpu.b == prev_state.b + 1);
    assert(cpu.pc == prev_state.pc + 1);

    return PASSED;
}

int main()
{
    int (*tests[])(void) =
    {
        when_noop_then_inc_pc_only,
        when_inc_b_then_b_incremented
    };
    int num_tests = sizeof(tests) / sizeof(void*);

    run_tests(tests, num_tests);

    return 0;
}
