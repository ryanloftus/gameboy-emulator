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

/* Opcodes that should hard-lock the CPU if encountered */
static bool is_invalid_opcode(uint8_t opcode)
{
    /* These opcodes are deliberately invalid and should crash the CPU */
    static const uint8_t invalid_opcodes[] =
    {
        0xD3, 0xDB, 0xDD,
        0xE3, 0xE4, 0xEB, 0xEC, 0xED,
        0xF4, 0xFC, 0xFD
    };

    for (size_t i = 0; i < sizeof(invalid_opcodes); ++i)
    {
        if (opcode == invalid_opcodes[i])
        {
            return true;
        }
    }
    return false;
}

void fetch_execute(virtual_cpu *cpu)
{
    debug_assert(cpu != NULL);
    debug_assert(cpu->code != NULL);

    uint8_t opcode = cpu->code[cpu->pc];

    debug_assert(!is_invalid_opcode(opcode));

    decoded_instr dec;

    if (!decode_opcode(opcode, &dec))
    {
        printf("unimplemented opcode %d\n", opcode);
        return;
    }

    execute_instruction(cpu, &dec);
    cpu->pc += dec.bytes;
    cpu->cycles += dec.cycles;
}
