#include "cpu.h"
#include "testing_utils.h"

#include <stdio.h>
#include <memory.h>

const int FAILED = 0;
const int PASSED = 1;

int when_noop_then_inc_pc_only()
{
    virtual_cpu cpu;
    virtual_cpu prev_state;
    uint8_t code = 0;

    create_virtual_cpu(&cpu);
    memcpy(&prev_state, &cpu, sizeof(virtual_cpu));

    fetch_execute(&cpu, &code);

    return cpu.a == prev_state.a &&
        cpu.b == prev_state.b &&
        cpu.c == prev_state.c &&
        cpu.d == prev_state.d &&
        cpu.e == prev_state.e &&
        cpu.f == prev_state.f &&
        cpu.h == prev_state.h &&
        cpu.l == prev_state.l &&
        cpu.sp == prev_state.sp &&
        cpu.pc == prev_state.pc + 1;
}

int main()
{
    printf("Running tests\n\n");

    int (*tests[])(void) = { when_noop_then_inc_pc_only };

    int total = 1;
    int passed = 0;
    int failed = 0;

    for (int i = 0; i < total; ++i)
    {
        if (tests[i]() == PASSED)
        {
            passed++;
        }
        else
        {
            failed++;
        }
    }

    printf("Passed: %d tests\n", passed);
    printf("Failed: %d tests\n", failed);
    printf("Total: %d\n", total);

    return 0;
}
