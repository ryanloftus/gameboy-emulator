#include "cpu.h"
#include "testing_utils.h"

#include <stdio.h>
#include <memory.h>

int test_noop()
{
    virtual_cpu cpu;
    uint8_t code = 0;

    create_virtual_cpu(&cpu);

    fetch_execute(&cpu, NULL, &code);

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

    fetch_execute(&cpu, NULL, code);

    assert(cpu.b == 0);
    assert(cpu.c == 1);
    assert(cpu.bc == 1);
    assert(cpu.pc == 1);

    fetch_execute(&cpu, NULL, code);

    assert(cpu.b == 1);
    assert(cpu.c == 1);
    assert(cpu.bc == (1 << 8) + 1);
    assert(cpu.pc == 2);

    return PASSED;
}

int test_dec_r8()
{
    virtual_cpu cpu;
    uint8_t code[] = {0b00001101, 0b00000101};

    create_virtual_cpu(&cpu);

    fetch_execute(&cpu, NULL, code);

    assert(cpu.b == 0);
    assert(cpu.c == 0xff);
    assert(cpu.bc == 0x00ff);
    assert(cpu.pc == 1);

    fetch_execute(&cpu, NULL, code);

    assert(cpu.b == 0xff);
    assert(cpu.c == 0xff);
    assert(cpu.bc == 0xffff);
    assert(cpu.pc == 2);
    
    uint8_t subtraction_flag = (cpu.f >> 6) & 1;
    assert(subtraction_flag == 1);

    return PASSED;
}

int test_inc_r16()
{
    virtual_cpu cpu;
    uint8_t code = 0b100011;

    create_virtual_cpu(&cpu);

    fetch_execute(&cpu, NULL, &code);

    assert(cpu.hl == 1);
    assert(cpu.pc == 1);

    return PASSED;
}

int test_dec_r16()
{
    virtual_cpu cpu;
    uint8_t code = 0b101011;

    create_virtual_cpu(&cpu);

    fetch_execute(&cpu, NULL, &code);

    assert(cpu.hl == 0b1111111111111111);
    assert(cpu.pc == 1);

    return PASSED;
}

int test_add_hl_r16()
{
    virtual_cpu cpu;
    uint8_t code = 0b011001;

    create_virtual_cpu(&cpu);
    cpu.de = 31;

    fetch_execute(&cpu, NULL, &code);

    assert(cpu.hl == 31);
    assert(cpu.pc == 1);

    return PASSED;
}

int test_zero_flag()
{
    virtual_cpu cpu;
    uint8_t code[] = {0b00001100, 0b00001101};

    create_virtual_cpu(&cpu);

    fetch_execute(&cpu, NULL, code);

    assert(cpu.f == 0);
    assert(cpu.c == 1);
    
    fetch_execute(&cpu, NULL, code);
    
    uint8_t zero_flag = cpu.f >> 7;
    assert(cpu.c == 0);
    assert(zero_flag == 1);

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
        test_add_hl_r16,
        test_zero_flag
    };
    int num_tests = sizeof(tests) / sizeof(void*);

    run_tests(tests, num_tests);

    return 0;
}
