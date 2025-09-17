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

    assert(cpu.af == prev_state.af);
    assert(cpu.bc == prev_state.bc);
    assert(cpu.de == prev_state.de);
    assert(cpu.hl == prev_state.hl);
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

    assert(cpu.bc == prev_state.bc + 1);
    assert(cpu.pc == prev_state.pc + 1);

    return PASSED;
}

int when_inc_c_then_c_incremented()
{
    virtual_cpu cpu;
    virtual_cpu prev_state;
    uint8_t code = 0b00001100;

    create_virtual_cpu(&cpu);
    memcpy(&prev_state, &cpu, sizeof(virtual_cpu));

    fetch_execute(&cpu, &code);

    assert(cpu.bc == prev_state.bc + (1 << 8));
    assert(cpu.pc == prev_state.pc + 1);

    return PASSED;
}

int when_dec_b_then_b_decremented()
{
    virtual_cpu cpu;
    virtual_cpu prev_state;
    uint8_t code = 0b00000101;

    create_virtual_cpu(&cpu);
    memcpy(&prev_state, &cpu, sizeof(virtual_cpu));

    fetch_execute(&cpu, &code);

    assert(cpu.bc == 0b11111111);
    assert(cpu.pc == prev_state.pc + 1);

    return PASSED;
}

int when_dec_c_then_c_decremented()
{
    virtual_cpu cpu;
    virtual_cpu prev_state;
    uint8_t code = 0b00001101;

    create_virtual_cpu(&cpu);
    memcpy(&prev_state, &cpu, sizeof(virtual_cpu));

    fetch_execute(&cpu, &code);

    assert(cpu.bc == 0b1111111100000000);
    assert(cpu.pc == prev_state.pc + 1);

    return PASSED;
}

int main()
{
    int (*tests[])(void) =
    {
        when_noop_then_inc_pc_only,
        when_inc_b_then_b_incremented,
        when_inc_c_then_c_incremented,
        when_dec_b_then_b_decremented,
        when_dec_c_then_c_decremented
    };
    int num_tests = sizeof(tests) / sizeof(void*);

    run_tests(tests, num_tests);

    return 0;
}
