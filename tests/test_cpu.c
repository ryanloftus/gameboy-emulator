#include "cpu.h"
#include "testing_utils.h"

#include <stdio.h>
#include <memory.h>

int when_noop_then_inc_pc_only()
{
    virtual_cpu cpu;
    uint8_t code = 0;

    create_virtual_cpu(&cpu);

    fetch_execute(&cpu, &code);

    assert(cpu.af == 0);
    assert(cpu.bc == 0);
    assert(cpu.de == 0);
    assert(cpu.hl == 0);
    assert(cpu.sp == 0);
    assert(cpu.pc == 1);

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

int when_inc_bc_then_bc_incremented()
{
    virtual_cpu cpu;
    uint8_t code = 0b000011;

    create_virtual_cpu(&cpu);

    fetch_execute(&cpu, &code);

    assert(cpu.bc == 1);
    assert(cpu.pc == 1);

    return PASSED;
}

int when_inc_hl_then_hl_incremented()
{
    virtual_cpu cpu;
    uint8_t code = 0b100011;

    create_virtual_cpu(&cpu);

    fetch_execute(&cpu, &code);

    assert(cpu.hl == 1);
    assert(cpu.pc == 1);

    return PASSED;
}

int when_dec_bc_then_bc_decremented()
{
    virtual_cpu cpu;
    uint8_t code = 0b001011;

    create_virtual_cpu(&cpu);

    fetch_execute(&cpu, &code);

    assert(cpu.bc == 0b1111111111111111);
    assert(cpu.pc == 1);

    return PASSED;
}

int when_dec_hl_then_hl_decremented()
{
    virtual_cpu cpu;
    uint8_t code = 0b101011;

    create_virtual_cpu(&cpu);

    fetch_execute(&cpu, &code);

    assert(cpu.hl == 0b1111111111111111);
    assert(cpu.pc == 1);

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
        when_dec_c_then_c_decremented,
        when_inc_bc_then_bc_incremented,
        when_inc_hl_then_hl_incremented,
        when_dec_bc_then_bc_decremented,
        when_dec_hl_then_hl_decremented
    };
    int num_tests = sizeof(tests) / sizeof(void*);

    run_tests(tests, num_tests);

    return 0;
}
