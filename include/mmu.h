#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdint.h>

#define B_128 0x80
#define B_256 0x100
#define KB_8 0x2000
#define KB_16 0x4000
#define KB_64 0x10000

#define INTERRUPT_ENABLE_REGISTER 0xFFFF

#define ROM_BANK_ZERO_START 0
#define ROM_BANK_ONE_START 0x4000
#define VIDEO_RAM_START 0x8000
#define EXTERNAL_RAM_START 0xA000
#define WORK_RAM_START 0xC000
#define ECHO_RAM_START 0xE000
#define OBJECT_ATTRIBUTE_MEMORY_START 0xFE00
#define PROHIBITED_MEMORY_START 0xFEA0
#define IO_REGISTERS_START 0xFF00
#define HIGH_RAM_START 0xFF80

typedef struct memory
{
    union
    {
        uint8_t raw[KB_64];
        struct
        {
            uint8_t rom_bank_zero[KB_16];
            uint8_t rom_bank_one[KB_16];
            uint8_t video_ram[KB_8];
            uint8_t external_ram[KB_8];
            uint8_t work_ram[KB_8];
            uint8_t echo_ram_ignore[0x1E00];
            uint8_t object_attribute_memory[0xA0];
            uint8_t prohibited_memory[0x60];
            uint8_t io_registers[B_128];
            uint8_t high_ram[B_128-1];
            uint8_t interrupt_enable_register;
        };
    };
} memory;

void init_memory(memory *mem);
uint8_t read_memory8(memory *mem, uint16_t addr);
uint16_t read_memory16(memory *mem, uint16_t addr);
void write_memory8(memory *mem, uint16_t addr, uint8_t value);
void write_memory16(memory *mem, uint16_t addr, uint16_t value);
uint8_t* access_memory8(memory *mem, uint16_t addr);

#endif
