#include "memory.h"

#include <memory.h>

/**
 * TODOs:
 * - echo ram
 * - asserts on prohibited ram
 * - bank switching
 */

void init_memory(memory *mem)
{
    memset(mem, 0, sizeof(memory));
}

uint8_t read_memory8(memory *mem, uint16_t addr)
{
    return mem->raw[addr];
}

uint16_t read_memory16(memory *mem, uint16_t addr)
{
    return (mem->raw[addr+1] << 8) | mem->raw[addr];
}

void write_memory8(memory *mem, uint16_t addr, uint8_t value)
{
    mem->raw[addr] = value;
}

void write_memory16(memory *mem, uint16_t addr, uint16_t value)
{
    uint8_t lowbyte = value & 0xff;
    uint8_t highbyte = value >> 8;
    mem->raw[addr] = lowbyte;
    mem->raw[addr+1] = highbyte;
}

uint8_t* access_memory8(memory *mem, uint16_t addr)
{
    return &(mem->raw[addr]);
}
