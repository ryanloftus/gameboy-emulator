#include "cpu.h"

void create_virtual_cpu(virtual_cpu *cpu)
{
    memset(cpu, 0, sizeof(virtual_cpu));
}

void fetch_execute(virtual_cpu *cpu, uint8_t *code)
{
    uint8_t opcode = code[cpu->pc];
}
