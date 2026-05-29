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

/** Interrupt vector addresses */
#define INT_VBLANK  0x40
#define INT_LCDC    0x48
#define INT_TIMER   0x50
#define INT_SERIAL  0x58
#define INT_JOYPAD  0x60

/* Priority order for interrupt servicing (bit position -> vector) */
static const uint8_t int_bits[] = { 0, 1, 2, 3, 4 };
static const uint16_t int_vectors[] = { INT_VBLANK, INT_LCDC, INT_TIMER, INT_SERIAL, INT_JOYPAD };

/**
 * Service the highest-priority pending interrupt if IME is set.
 * Clears the corresponding IF bit, clears IME (0xFFFF), and pushes
 * the current PC to the stack before jumping to the vector address.
 * Also wakes the CPU from halt.
 *
 * Returns 1 if an interrupt was serviced, 0 otherwise.
 */
static uint8_t service_interrupts(virtual_cpu *cpu)
{
    memory *mem = cpu->mem;

    uint8_t ie = mem->interrupt_enable_register;
    if (ie == 0) {
        return 0;
    }

    uint8_t iflag = mem->io_registers[IF_REG_ADDR & 0xFF];
    uint8_t pending = ie & iflag;

    if (pending == 0) {
        return 0;
    }

    /* Wake from halt */
    cpu->is_halted = 0;

    /* Find the highest-priority pending interrupt (lowest bit = highest priority) */
    uint8_t bit;
    for (bit = 0; bit < 5; bit++) {
        if (pending & (1u << int_bits[bit])) {
            break;
        }
    }
    if (bit >= 5) {
        return 0; /* should not happen */
    }

    /* Clear the IF bit for this interrupt */
    iflag &= (uint8_t)~(1u << int_bits[bit]);
    mem->io_registers[IF_REG_ADDR & 0xFF] = iflag;

    /* Disable IME */
    mem->interrupt_enable_register = 0;
    cpu->ei_scheduled = 0;

    /* Push PC onto stack */
    cpu->sp -= 2;
    write_memory8(mem, cpu->sp,     cpu->pc & 0xFF);
    write_memory8(mem, cpu->sp + 1, cpu->pc >> 8);

    /* Jump to interrupt vector */
    cpu->pc = int_vectors[bit];

    return 1;
}

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

    /* Service interrupts: if IME is set and there's a pending interrupt,
     * push PC, jump to vector, and wake from halt. This consumes the
     * instruction slot (no normal instruction is executed). */
    if (service_interrupts(cpu)) {
        /* 5-cycle "M1" for interrupt servicing — account for it */
        cpu->cycles += 5;
        /* No timers update here — timers are updated when the *next*
         * fetch_execute runs; the 5 M-cycles will be accounted then */
        return;
    }

    /* If halted and no interrupt was serviced, skip instruction execution.
     * The CPU just "nops" through cycles until an interrupt un-halts it. */
    if (cpu->is_halted) {
        /* Each fetch_execute call while halted consumes 1 M-cycle worth
         * of time so timers still advance */
        cpu->cycles += 1;
        update_timers(cpu, 1);
        return;
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

    if (g_debug_mode) {
        printf("executing instr %d\n", dec.id);
    }

    execute_instruction(cpu, &dec);
    cpu->pc += dec.bytes;
    cpu->cycles += dec.cycles;
    update_timers(cpu, dec.cycles);
}
