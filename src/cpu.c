#include "cpu.h"
#include "decode.h"
#include "instructions.h"
#include "debug.h"

#include <stdio.h>
#include <string.h>

/* DIV counter bit indices selected by TAC clock bits 1-0 */
static const uint8_t div_bit[] = { 9, 3, 5, 7 };

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
 * Return the bitmask of enabled, pending interrupts (IE & IF), or 0 if none.
 */
static uint8_t get_pending_interrupts(virtual_cpu *cpu)
{
    memory *mem = cpu->mem;
    uint8_t ie = mem->interrupt_enable_register;
    if (ie == 0) {
        return 0;
    }

    uint8_t iflag = mem->io_registers[IF_REG_ADDR & 0xFF];
    return ie & iflag;
}

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

    if (!cpu->ime) {
        return 0;
    }

    uint8_t pending = get_pending_interrupts(cpu);
    if (pending == 0) {
        return 0;
    }

    uint8_t iflag = mem->io_registers[IF_REG_ADDR & 0xFF];

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
    cpu->ime = 0;
    cpu->ei_scheduled = 0;

    /* Push PC onto stack */
    cpu->sp -= 2;
    write_memory8(mem, cpu->sp,     cpu->pc & 0xFF);
    write_memory8(mem, cpu->sp + 1, cpu->pc >> 8);

    /* Jump to interrupt vector */
    cpu->pc = int_vectors[bit];

    return 1;
}

static void tima_increment(memory *mem)
{
    uint8_t tima = mem->io_registers[TIMA_REG_ADDR & 0xFF];
    if (tima == 0xFF) {
        mem->io_registers[TIMA_REG_ADDR & 0xFF] = mem->io_registers[TMA_REG_ADDR & 0xFF];
        mem->io_registers[IF_REG_ADDR & 0xFF] |= 0x04;
        return;
    }

    mem->io_registers[TIMA_REG_ADDR & 0xFF] = (uint8_t)(tima + 1);
}

static bool div_bit_is_set(uint16_t div, uint8_t clock_select)
{
    return (div & (uint16_t)(1u << div_bit[clock_select])) != 0;
}

static void tima_edge_tick(memory *mem, uint16_t old_div, uint16_t new_div, uint8_t tac)
{
    if ((tac & 0x04) == 0) {
        return;
    }

    uint8_t clock_select = tac & 0x03;
    uint16_t mask = (uint16_t)(1u << div_bit[clock_select]);
    if ((old_div & mask) != 0 && (new_div & mask) == 0) {
        tima_increment(mem);
    }
}

void tima_on_tac_write(memory *mem, uint8_t old_tac, uint8_t new_tac)
{
    /* Changing the selected DIV bit from 1 to 0 triggers a TIMA increment. */
    if ((old_tac & 0x04) == 0 && (new_tac & 0x04) == 0) {
        return;
    }

    uint8_t old_clock = old_tac & 0x03;
    uint8_t new_clock = new_tac & 0x03;
    uint16_t div = mem->div_counter;

    if (div_bit_is_set(div, old_clock) && !div_bit_is_set(div, new_clock)) {
        tima_increment(mem);
    }
}

void tima_on_div_write(memory *mem)
{
    uint8_t tac = mem->io_registers[TAC_REG_ADDR & 0xFF];
    uint16_t old_div = mem->div_counter;
    mem->div_counter = 0;
    tima_edge_tick(mem, old_div, 0, tac);
}

void update_timers(virtual_cpu *cpu, uint16_t cycles_elapsed)
{
    memory *mem = cpu->mem;

    for (uint16_t m = 0; m < cycles_elapsed; m++) {
        update_oam_dma(mem);
        uint16_t old_div = mem->div_counter;
        mem->div_counter += 4;
        tima_edge_tick(mem, old_div, mem->div_counter, mem->io_registers[TAC_REG_ADDR & 0xFF]);
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

static void run_instruction(virtual_cpu *cpu, decoded_instr *dec)
{
    if (dec->cycles == 0) {
        execute_instruction(cpu, dec);
        return;
    }

    uint64_t cycles_before = cpu->cycles;

    /* IO reads happen on the last M-cycle; tick the timer for earlier cycles first. */
    if (dec->cycles > 1) {
        update_timers(cpu, (uint16_t)(dec->cycles - 1));
    }

    execute_instruction(cpu, dec);
    update_timers(cpu, 1);

    /* Conditional branches/returns add a cycle in execute_instruction. */
    uint16_t extra = (uint16_t)(cpu->cycles - cycles_before);
    if (extra > 0) {
        update_timers(cpu, extra);
    }
}

void fetch_execute(virtual_cpu *cpu)
{
    debug_assert(cpu != NULL);
    debug_assert(cpu->mem != NULL);

    if (service_interrupts(cpu)) {
        cpu->cycles += 5;
        update_timers(cpu, 5);
        return;
    }

    if (cpu->ei_scheduled)
    {
        cpu->ime = 1;
        cpu->ei_scheduled = 0;
    }

    if (cpu->is_halted) {
        if (!cpu->ime && get_pending_interrupts(cpu)) {
            cpu->is_halted = 0;
        } else {
            cpu->cycles += 1;
            update_timers(cpu, 1);
            return;
        }
    }

    uint8_t opcode = read_memory8(cpu->mem, cpu->pc);

    debug_assert(!is_invalid_opcode(opcode));

    decoded_instr dec;

    if (opcode == 0xCB)
    {
        uint8_t cb_opcode = read_memory8(cpu->mem, cpu->pc + 1);

        if (!decode_cb_opcode(cb_opcode, &dec))
        {
            if (g_instr_log) {
                debug_log_printf("unimplemented CB opcode %d\n", cb_opcode);
            } else {
                printf("unimplemented CB opcode %d\n", cb_opcode);
            }
            return;
        }

        if (g_instr_log) {
            debug_log_instruction(cpu->pc, &dec, cpu->mem);
        }

        run_instruction(cpu, &dec);
        cpu->pc += dec.bytes;
        cpu->cycles += dec.cycles;
        return;
    }

    if (!decode_opcode(opcode, &dec))
    {
        if (g_instr_log) {
            debug_log_printf("unimplemented opcode %d\n", opcode);
        } else {
            printf("unimplemented opcode %d\n", opcode);
        }
        return;
    }

    if (g_instr_log) {
        debug_log_instruction(cpu->pc, &dec, cpu->mem);
    }

    run_instruction(cpu, &dec);
    cpu->pc += dec.bytes;
    cpu->cycles += dec.cycles;
}
