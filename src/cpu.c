#include "cpu.h"
#include "decode.h"
#include "instructions.h"
#include "debug.h"

#include <stdio.h>
#include <string.h>

/* Timer clock thresholds (T-cycles per TIMA increment) for TAC bits 1-0 */
static const uint16_t tima_thresholds[] = {
    1024,   /* 00: 4096 Hz */
    16,     /* 01: 262144 Hz */
    64,     /* 10: 65536 Hz */
    256     /* 11: 16384 Hz */
};

void update_timers(virtual_cpu *cpu, uint16_t cycles_elapsed)
{
    memory *mem = cpu->mem;
    uint8_t tac = mem->io_registers[TAC_REG_ADDR & 0xFF];

    /* Update the internal 16-bit divider counter */
    mem->div_counter += cycles_elapsed;

    /* Update the TIMA counter */
    if (tac & 0x04) {
        uint8_t clock_select = tac & 0x03;
        uint16_t threshold = tima_thresholds[clock_select];

        mem->tima_accum += cycles_elapsed;
        while (mem->tima_accum >= threshold) {
            mem->tima_accum -= threshold;

            /* Increment TIMA */
            uint8_t tima = mem->io_registers[TIMA_REG_ADDR & 0xFF];
            tima++;
            if (tima == 0) {
                /* Overflow: reload from TMA and request timer interrupt */
                tima = mem->io_registers[TMA_REG_ADDR & 0xFF];
                mem->io_registers[IF_REG_ADDR & 0xFF] |= 0x04; /* set timer interrupt flag (bit 2) */
            }
            mem->io_registers[TIMA_REG_ADDR & 0xFF] = tima;
        }
    }
}

void create_virtual_cpu(virtual_cpu *cpu, memory *mem)
{
    memset(cpu, 0, sizeof(virtual_cpu));
    cpu->mem = mem;
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
    debug_assert(cpu->mem != NULL);

    /* Apply delayed EI: if EI was scheduled, enable IME now at the start of
       the next instruction (this is the "one instruction delay") */
    if (cpu->ei_scheduled)
    {
        write_memory8(cpu->mem, 0xFFFF, 1);
        cpu->ei_scheduled = 0;
    }

    uint8_t opcode = read_memory8(cpu->mem, cpu->pc);

    debug_assert(!is_invalid_opcode(opcode));

    decoded_instr dec;

    /* Handle CB prefix */
    if (opcode == 0xCB)
    {
        uint8_t cb_opcode = read_memory8(cpu->mem, cpu->pc + 1);

        if (!decode_cb_opcode(cb_opcode, &dec))
        {
            printf("unimplemented CB opcode %d\n", cb_opcode);
            return;
        }

        execute_instruction(cpu, &dec);
        cpu->pc += dec.bytes;
        cpu->cycles += dec.cycles;
        update_timers(cpu, dec.cycles);
        return;
    }

    if (!decode_opcode(opcode, &dec))
    {
        printf("unimplemented opcode %d\n", opcode);
        return;
    }

    printf("executing instr %d\n", dec.id);

    execute_instruction(cpu, &dec);
    cpu->pc += dec.bytes;
    cpu->cycles += dec.cycles;
    update_timers(cpu, dec.cycles);
}
