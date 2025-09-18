#include "cpu.h"
#include "testing_utils.h"

#include <stdio.h>
#include <memory.h>

int test_noop()
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

int test_inc_r8()
{
    virtual_cpu cpu;
    uint8_t code[] = {0b00001100, 0b00000100};

    create_virtual_cpu(&cpu);

    fetch_execute(&cpu, code);

    assert(cpu.bc == (1 << 8));
    assert(cpu.pc == 1);

    fetch_execute(&cpu, code);

    assert(cpu.bc == (1 << 8) + 1);
    assert(cpu.pc == 2);

    return PASSED;
}

int test_dec_r8()
{
    virtual_cpu cpu;
    uint8_t code[] = {0b00001101, 0b00000101};

    create_virtual_cpu(&cpu);

    fetch_execute(&cpu, code);

    assert(cpu.bc == 0b1111111100000000);
    assert(cpu.pc == 1);

    fetch_execute(&cpu, code);

    assert(cpu.bc == 0b1111111111111111);
    assert(cpu.pc == 2);

    return PASSED;
}

int test_inc_r16()
{
    virtual_cpu cpu;
    uint8_t code = 0b100011;

    create_virtual_cpu(&cpu);

    fetch_execute(&cpu, &code);

    assert(cpu.hl == 1);
    assert(cpu.pc == 1);

    return PASSED;
}

int test_dec_r16()
{
    virtual_cpu cpu;
    uint8_t code = 0b101011;

    create_virtual_cpu(&cpu);

    fetch_execute(&cpu, &code);

    assert(cpu.hl == 0b1111111111111111);
    assert(cpu.pc == 1);

    return PASSED;
}

int test_add_hl_r16()
{
    return PASSED;
}

int main()
{
    int (*tests[])(void) =
    {
        test_noop,
        test_inc_r8,
        test_dec_r8,
        test_inc_r16,
        test_dec_r16,
        test_add_hl_r16
    };
    int num_tests = sizeof(tests) / sizeof(void*);

    run_tests(tests, num_tests);

    return 0;
}
