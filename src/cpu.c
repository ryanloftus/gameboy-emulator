#include "cpu.h"
#include "decode.h"
#include "instructions.h"
#include "debug.h"

#include <stdio.h>
#include <string.h>

void create_virtual_cpu(virtual_cpu *cpu, memory *mem, uint8_t *code)
{
    memset(cpu, 0, sizeof(virtual_cpu));
    cpu->mem = mem;
    cpu->code = code;
}

void fetch_execute(virtual_cpu *cpu)
{
    debug_assert(cpu != NULL);
    debug_assert(cpu->code != NULL);

    uint8_t opcode = cpu->code[cpu->pc];
    decoded_instr dec;

    if (!decode_opcode(opcode, &dec))
    {
        printf("unimplemented opcode %d\n", opcode);
        return;
    }

    execute_instruction(cpu, &dec);
    cpu->pc += dec.bytes;
}
