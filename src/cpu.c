#include "cpu.h"

#include <stdio.h>
#include <memory.h>

void noop()
{
    return;
}

void create_virtual_cpu(virtual_cpu *cpu)
{
    memset(cpu, 0, sizeof(virtual_cpu));
}

void fetch_execute(virtual_cpu *cpu, uint8_t *code)
{
    uint8_t opcode = code[cpu->pc];

    switch (opcode)
    {
        case 0:
            noop();
            cpu->pc += 1;
            break;
        default:
            printf("unimplemented opcode %d\n", opcode);
            break;
    }
}
