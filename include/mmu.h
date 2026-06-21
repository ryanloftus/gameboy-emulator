#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdint.h>
#include <stdlib.h>

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

/* Timer registers */
#define DIV_REG_ADDR  0xFF04
#define TIMA_REG_ADDR 0xFF05
#define TMA_REG_ADDR  0xFF06
#define TAC_REG_ADDR  0xFF07

/* Interrupt flag register */
#define IF_REG_ADDR   0xFF0F

/* Serial communication registers */
#define SB_REG_ADDR   0xFF01  /* Serial transfer data */
#define SC_REG_ADDR   0xFF02  /* Serial control */

#define ROM_BANK_SIZE 0x4000
#define EXT_RAM_BANK_SIZE 0x2000

/* Cartridge header addresses */
#define CART_HEADER_ROM_SIZE_ADDR 0x0148
#define CART_HEADER_RAM_SIZE_ADDR 0x0149

/* MBC1 register address ranges */
#define MBC1_RAM_ENABLE_START  0x0000
#define MBC1_RAM_ENABLE_END    0x1FFF
#define MBC1_ROM_BANK_START    0x2000
#define MBC1_ROM_BANK_END      0x3FFF
#define MBC1_RAM_BANK_START    0x4000
#define MBC1_RAM_BANK_END      0x5FFF
#define MBC1_MODE_SELECT_START 0x6000
#define MBC1_MODE_SELECT_END   0x7FFF

/* Shared MBC register address ranges (MBC3/MBC5) */
#define MBC_RAM_ENABLE_START   0x0000
#define MBC_RAM_ENABLE_END     0x1FFF
#define MBC_ROM_BANK_START     0x2000
#define MBC_ROM_BANK_END       0x3FFF
#define MBC_RAM_BANK_START     0x4000
#define MBC_RAM_BANK_END       0x5FFF
#define MBC_LATCH_CLOCK_START  0x6000
#define MBC_LATCH_CLOCK_END    0x7FFF

/* MBC5-specific ROM bank high bit register */
#define MBC5_ROM_BANK_HI_START 0x3000
#define MBC5_ROM_BANK_HI_END   0x3FFF

/* MBC2 built-in RAM: 512 x 4-bit values */
#define MBC2_RAM_SIZE          512

/* MBC3 RTC register indices (selected via RAM bank register 0x08-0x0C) */
#define MBC3_RTC_SECONDS       0
#define MBC3_RTC_MINUTES       1
#define MBC3_RTC_HOURS         2
#define MBC3_RTC_DAY_LOW       3
#define MBC3_RTC_DAY_HIGH      4
#define MBC3_RTC_REG_COUNT     5

/* ROM sizes from header byte 0x0148 */
static const size_t rom_sizes[] = {
    2 * ROM_BANK_SIZE,     /* 00: 32 KB */
    4 * ROM_BANK_SIZE,     /* 01: 64 KB */
    8 * ROM_BANK_SIZE,     /* 02: 128 KB */
    16 * ROM_BANK_SIZE,    /* 03: 256 KB */
    32 * ROM_BANK_SIZE,    /* 04: 512 KB */
    64 * ROM_BANK_SIZE,    /* 05: 1 MB */
    128 * ROM_BANK_SIZE,   /* 06: 2 MB */
    256 * ROM_BANK_SIZE,   /* 07: 4 MB */
    512 * ROM_BANK_SIZE,   /* 08: 8 MB */
};

/* RAM sizes from header byte 0x0149 */
static const size_t ram_sizes[] = {
    0,      /* 00: None */
    2 * 1024, /* 01: 2 KB */
    8 * 1024, /* 02: 8 KB */
    32 * 1024,/* 03: 32 KB (4 banks) */
    128 * 1024,/* 04: 128 KB (16 banks) */
    64 * 1024,/* 05: 64 KB (8 banks) */
};

/* Number of RAM banks for each header RAM size code */
static const uint8_t ram_bank_counts[] = {
    0,  /* 00: None */
    1,  /* 01: 2 KB (only 1 bank-ish, but treated as contiguous) */
    1,  /* 02: 8 KB */
    4,  /* 03: 32 KB (4 banks) */
    16, /* 04: 128 KB (16 banks) */
    8,  /* 05: 64 KB (8 banks) */
};

typedef struct cartridge
{
    /* ROM */
    uint8_t *rom;
    size_t rom_size;

    /* RAM */
    uint8_t *ram;
    size_t ram_size;

    /* MBC type from header */
    uint8_t mbc_type;

    /* Shared MBC registers */
    uint8_t ram_enable;      /* RAM enable flag (enabled when == 0x0A) */
    uint8_t bank_reg_1;      /* MBC1: ROM lower 5 bits; MBC3: ROM lower 7 bits; MBC5: ROM lower 8 bits */
    uint8_t bank_reg_2;      /* MBC1: upper ROM/RAM bits; MBC3: RAM/RTC bank select; MBC5: RAM bank (+ rumble bit) */
    uint8_t banking_mode;    /* MBC1 only: 0 = ROM mode, 1 = RAM mode */
    uint8_t rom_bank_hi;     /* MBC5 only: 9th bit of ROM bank number */

    /* MBC3 RTC */
    uint8_t rtc_regs[MBC3_RTC_REG_COUNT];
    uint8_t rtc_latched[MBC3_RTC_REG_COUNT];
    uint8_t rtc_latch_ready;

    /* Computed current banks */
    uint16_t current_rom_bank;
    uint8_t current_ram_bank;
} cartridge;

typedef struct memory
{
    cartridge cartridge;
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

    /* Internal timer state (not memory-mapped) */
    uint16_t div_counter;   /* Internal 16-bit divider counter, incremented every T-cycle */
} memory;

void init_memory(memory *mem, const char *rom_path);
void destroy_memory(memory *mem);
uint8_t read_memory8(memory *mem, uint16_t addr);
uint16_t read_memory16(memory *mem, uint16_t addr);
void write_memory8(memory *mem, uint16_t addr, uint8_t value);
void write_memory16(memory *mem, uint16_t addr, uint16_t value);
uint8_t* access_memory8(memory *mem, uint16_t addr);

#endif