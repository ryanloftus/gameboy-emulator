#include "mmu.h"
#include "debug.h"

#include <memory.h>

/**
 * Internal: redirect echo RAM (0xE000–0xFDFF) to the corresponding
 * work RAM location (0xC000–0xDDFF), and assert on prohibited
 * addresses (0xFEA0–0xFEFF).
 */
static uint16_t sanitize_addr(uint16_t addr)
{
    if (addr >= 0xFEA0 && addr <= 0xFEFF) {
        debug_assert(0 && "access to prohibited memory region 0xFEA0-0xFEFF");
    }

    if (addr >= 0xE000 && addr <= 0xFDFF) {
        addr -= 0x2000;  /* echo → work RAM */
    }

    return addr;
}

void init_memory(memory *mem)
{
    memset(mem, 0, sizeof(memory));
}

uint8_t read_memory8(memory *mem, uint16_t addr)
{
    addr = sanitize_addr(addr);
    return mem->raw[addr];
}

uint16_t read_memory16(memory *mem, uint16_t addr)
{
    uint8_t low = read_memory8(mem, addr);
    uint8_t high = read_memory8(mem, addr + 1);
    return (high << 8) | low;
}

void write_memory8(memory *mem, uint16_t addr, uint8_t value)
{
    addr = sanitize_addr(addr);
    mem->raw[addr] = value;
}

void write_memory16(memory *mem, uint16_t addr, uint16_t value)
{
    write_memory8(mem, addr, value & 0xFF);
    write_memory8(mem, addr + 1, (value >> 8) & 0xFF);
}

uint8_t* access_memory8(memory *mem, uint16_t addr)
{
    addr = sanitize_addr(addr);
    return &(mem->raw[addr]);
}
