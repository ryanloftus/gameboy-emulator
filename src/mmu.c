#include "mmu.h"
#include "debug.h"

#include <memory.h>
#include <stdbool.h>
#include <stdio.h>

#define MBC_TYPE_ADDR   0x0147
#define RAM_SIZE_ADDR   0x0149

/* External RAM bank size (8 KB per bank) */
#define EXT_RAM_BANK_SIZE 0x2000

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

/**
 * Returns true if addr falls in a region that is backed (or potentially
 * backed) by cartridge hardware: ROM banks 0/1 (0x0000–0x7FFF) and
 * external RAM (0xA000–0xBFFF).
 */
static bool is_cartridge_backed_addr(uint16_t addr)
{
    return addr < VIDEO_RAM_START ||
           (addr >= EXTERNAL_RAM_START && addr < WORK_RAM_START);
}

/* ------------------------------------------------------------------ */
/*  MBC1 helpers                                                      */
/* ------------------------------------------------------------------ */

/**
 * Compute the effective ROM bank number for the 0x4000–0x7FFF window.
 *
 * The 5-bit bank_reg_1 selects the bank, with 0 mapped to 1.
 * In ROM mode, bank_reg_2 provides the upper 2 bits (bits 6-5).
 * In RAM mode, bank_reg_2 is used only for RAM banking, so only the
 * lower 5 bits from bank_reg_1 are used.
 *
 * The result is wrapped (modulo) to the number of ROM banks present.
 */
static uint8_t mbc1_rom_bank_1(cartridge *cart)
{
    uint8_t effective = (cart->bank_reg_1 == 0) ? 1 : cart->bank_reg_1;

    uint8_t bank_num;
    if (cart->banking_mode == 0) {
        /* ROM mode: bank_reg_2 provides upper bits 6-5 */
        bank_num = (cart->bank_reg_2 << 5) | effective;
    } else {
        /* RAM mode: only the lower 5 bits from bank_reg_1 matter */
        bank_num = effective;
    }

    uint8_t num_banks = (uint8_t)(cart->rom_size / ROM_BANK_SIZE);
    if (num_banks > 0) {
        bank_num = bank_num % num_banks;
    }
    return bank_num;
}

/**
 * Compute the effective physical ROM bank mapped to the 0x0000–0x3FFF window.
 *
 * In ROM mode (banking_mode == 0): always physical bank 0.
 * In RAM mode (banking_mode == 1):
 *   - If bank_reg_2 == 0: physical bank 0.
 *   - If bank_reg_2 != 0: physical bank (bank_reg_2 << 5).
 */
static uint8_t mbc1_rom_bank_0(cartridge *cart)
{
    if (cart->banking_mode == 0) {
        return 0;
    }

    /* RAM mode */
    if (cart->bank_reg_2 == 0) {
        return 0;
    }

    uint8_t bank_num = cart->bank_reg_2 << 5;
    uint8_t num_banks = (uint8_t)(cart->rom_size / ROM_BANK_SIZE);
    if (num_banks > 0) {
        bank_num = bank_num % num_banks;
    }
    return bank_num;
}

/**
 * Compute the current external RAM bank number.
 *
 * In ROM mode: always bank 0 (bank_reg_2 is used as upper ROM bits).
 * In RAM mode: bank_reg_2 selects the RAM bank (lower 2 bits, 0–3).
 *
 * RAM bank number is wrapped to the number of RAM banks available.
 */
static uint8_t mbc1_ram_bank(cartridge *cart)
{
    uint8_t bank = (cart->banking_mode == 0) ? 0 : (cart->bank_reg_2 & 0x03);

    /* Determine number of RAM banks */
    uint8_t num_banks = (uint8_t)(cart->ram_size / EXT_RAM_BANK_SIZE);
    if (num_banks > 1) {
        bank = bank % num_banks;
    } else if (num_banks == 0) {
        bank = 0;
    }
    return bank;
}

/**
 * Update the cached bank numbers in the cartridge struct.
 * Called after any MBC1 register change.
 */
static void mbc1_recalculate_banks(cartridge *cart)
{
    cart->current_rom_bank = mbc1_rom_bank_1(cart);
    cart->current_ram_bank = mbc1_ram_bank(cart);
}

/**
 * Handle a write to MBC1 register space (0x0000 – 0x7FFF).
 */
static void mbc1_write_register(memory *mem, uint16_t addr, uint8_t value)
{
    cartridge *cart = &mem->cartridge;

    if (addr <= MBC1_RAM_ENABLE_END) {
        /* 0x0000 – 0x1FFF: RAM Enable */
        cart->ram_enable = (value == 0x0A) ? 0x0A : 0x00;
        return;
    }

    if (addr >= MBC1_ROM_BANK_START && addr <= MBC1_ROM_BANK_END) {
        /* 0x2000 – 0x3FFF: ROM Bank Number (lower 5 bits) */
        cart->bank_reg_1 = value & 0x1F;
        mbc1_recalculate_banks(cart);
        return;
    }

    if (addr >= MBC1_RAM_BANK_START && addr <= MBC1_RAM_BANK_END) {
        /* 0x4000 – 0x5FFF: RAM Bank Number / Upper ROM Bank Bits */
        cart->bank_reg_2 = value & 0x03;
        mbc1_recalculate_banks(cart);
        return;
    }

    if (addr >= MBC1_MODE_SELECT_START && addr <= MBC1_MODE_SELECT_END) {
        /* 0x6000 – 0x7FFF: Mode Select (ROM mode = 0, RAM mode = 1) */
        cart->banking_mode = value & 0x01;
        mbc1_recalculate_banks(cart);
        return;
    }
}

/* ------------------------------------------------------------------ */
/*  Cartridge loading                                                 */
/* ------------------------------------------------------------------ */

static void load_cartridge(cartridge *cartridge, const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        debug_assert(0 && "failed to open ROM file");
        return;
    }

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (file_size <= 0) {
        debug_assert(0 && "ROM file is empty");
        fclose(f);
        return;
    }

    cartridge->rom = malloc((size_t)file_size);
    if (!cartridge->rom) {
        debug_assert(0 && "malloc failed for ROM data");
        fclose(f);
        return;
    }

    size_t bytes_read = fread(cartridge->rom, 1, (size_t)file_size, f);
    fclose(f);

    if ((long)bytes_read != file_size) {
        debug_assert(0 && "failed to read ROM file fully");
        free(cartridge->rom);
        cartridge->rom = NULL;
        return;
    }

    cartridge->rom_size = (size_t)file_size;

    /* Parse MBC type from cartridge header */
    if (cartridge->rom_size > MBC_TYPE_ADDR) {
        cartridge->mbc_type = cartridge->rom[MBC_TYPE_ADDR];
        if (g_debug_mode) {
            printf("ROM has MBC type %d\n", cartridge->mbc_type);
        }
        debug_assert(cartridge->mbc_type == 0 || cartridge->mbc_type == 1);
    }

    /* Parse RAM size from cartridge header and allocate RAM */
    if (cartridge->rom_size > RAM_SIZE_ADDR) {
        uint8_t ram_size_code = cartridge->rom[RAM_SIZE_ADDR];
        size_t ram_count = sizeof(ram_sizes) / sizeof(ram_sizes[0]);
        if (ram_size_code < ram_count) {
            cartridge->ram_size = ram_sizes[ram_size_code];
        } else {
            cartridge->ram_size = 0;
        }
    } else {
        cartridge->ram_size = 0;
    }

    if (cartridge->ram_size > 0) {
        cartridge->ram = calloc(1, cartridge->ram_size);
        if (!cartridge->ram) {
            debug_assert(0 && "calloc failed for cartridge RAM");
            cartridge->ram_size = 0;
        }
    }
}

void destroy_memory(memory *mem)
{
    free(mem->cartridge.rom);
    mem->cartridge.rom = NULL;
    mem->cartridge.rom_size = 0;

    free(mem->cartridge.ram);
    mem->cartridge.ram = NULL;
    mem->cartridge.ram_size = 0;

    mem->cartridge.mbc_type = 0;
}

void init_memory(memory *mem, const char *rom_path)
{
    memset(mem, 0, sizeof(memory));

    if (rom_path != NULL) {
        load_cartridge(&mem->cartridge, rom_path);
    }

    /* Recalculate MBC bank numbers based on initialized register values.
     * After memset, bank_reg_1 = 0 which should map to ROM bank 1
     * (bank 0 is reserved for the 0x0000-0x3FFF window). Without this,
     * current_rom_bank stays at 0, incorrectly mapping the 0x4000-0x7FFF
     * window to physical bank 0. */
    if (mem->cartridge.mbc_type == 1) {
        mbc1_recalculate_banks(&mem->cartridge);
    }
}

/* ------------------------------------------------------------------ */
/*  Memory reads & writes                                             */
/* ------------------------------------------------------------------ */

uint8_t read_memory8(memory *mem, uint16_t addr)
{
    /* DIV register returns upper 8 bits of internal divider counter */
    if (addr == DIV_REG_ADDR) {
        return (uint8_t)(mem->div_counter >> 8);
    }

    if (is_cartridge_backed_addr(addr)) {
        cartridge *cart = &mem->cartridge;

        if (cart->mbc_type == 1) {
            if (addr < ROM_BANK_ONE_START) {
                /* ROM bank 0: 0x0000 – 0x3FFF (may include multi-game
                 * bank 0 remapping in RAM mode) */
                uint8_t bank = mbc1_rom_bank_0(cart);
                size_t offset = (size_t)bank * ROM_BANK_SIZE + (uint32_t)addr;
                if (offset < cart->rom_size) {
                    return cart->rom[offset];
                }
                return 0xFF;
            }

            if (addr < VIDEO_RAM_START) {
                /* ROM bank 1: 0x4000 – 0x7FFF */
                uint8_t bank = cart->current_rom_bank;
                size_t offset = (size_t)bank * ROM_BANK_SIZE + (uint32_t)(addr - ROM_BANK_ONE_START);
                if (offset < cart->rom_size) {
                    return cart->rom[offset];
                }
                return 0xFF;
            }

            /* External RAM: 0xA000 – 0xBFFF */
            if (cart->ram_enable == 0x0A && cart->ram != NULL) {
                size_t bank_offset = (size_t)cart->current_ram_bank * EXT_RAM_BANK_SIZE;
                size_t offset = bank_offset + (addr - EXTERNAL_RAM_START);
                if (offset < cart->ram_size) {
                    return cart->ram[offset];
                }
            }
            return 0xFF;
        }

        /* MBC0 (no mapper) path */
        if (addr < cart->rom_size) {
            return cart->rom[addr];
        }
        return 0xFF;
    }

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
    /* Writing any value to DIV resets the internal divider counter */
    if (addr == DIV_REG_ADDR) {
        mem->div_counter = 0;
        return;
    }

    if (is_cartridge_backed_addr(addr)) {
        cartridge *cart = &mem->cartridge;

        if (cart->mbc_type == 1) {
            if (addr < VIDEO_RAM_START) {
                /* MBC1 register writes: 0x0000 – 0x7FFF */
                mbc1_write_register(mem, addr, value);
                return;
            }

            /* External RAM write: 0xA000 – 0xBFFF */
            if (cart->ram_enable == 0x0A && cart->ram != NULL) {
                size_t bank_offset = (size_t)cart->current_ram_bank * EXT_RAM_BANK_SIZE;
                size_t offset = bank_offset + (addr - EXTERNAL_RAM_START);
                if (offset < cart->ram_size) {
                    cart->ram[offset] = value;
                }
            }
            return;
        }

        /* MBC0 — writes to ROM space are ignored (ROM is read-only) */
        return;
    }

    /* Serial transfer: when SC ($FF02) is written with 0x81, the
     * character in SB ($FF01) is ready to be transmitted */
    if (addr == SC_REG_ADDR && value == 0x81) {
        putchar(mem->raw[SB_REG_ADDR]);
        fflush(stdout);
    }

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
    if (is_cartridge_backed_addr(addr)) {
        /* For MBC0 with a simple linear ROM, return the direct pointer.
         * For MBC1, bank switching makes pointer-based access unreliable,
         * so we assert. */
        cartridge *cart = &mem->cartridge;
        if (cart->mbc_type == 1) {
            debug_assert(0 && "access_memory8 on MBC1 cartridge address not supported");
            return NULL;
        }

        /* MBC0: return direct pointer if within ROM range, otherwise assert */
        if (addr < cart->rom_size) {
            return &cart->rom[addr];
        }

        debug_assert(0 && "access_memory8 called on cartridge address with no backing data");
        return NULL;
    }

    addr = sanitize_addr(addr);
    return &(mem->raw[addr]);
}