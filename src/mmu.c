#include "mmu.h"
#include "debug.h"

#include <memory.h>
#include <stdbool.h>
#include <stdio.h>

#define MBC_TYPE_ADDR   0x0147
#define RAM_SIZE_ADDR   0x0149

/* External RAM bank size (8 KB per bank) */
#define EXT_RAM_BANK_SIZE 0x2000

/* ------------------------------------------------------------------ */
/*  MBC type helpers                                                  */
/* ------------------------------------------------------------------ */

static bool is_mbc1(uint8_t type)
{
    return type >= 0x01 && type <= 0x03;
}

static bool is_mbc2(uint8_t type)
{
    return type == 0x05 || type == 0x06;
}

static bool is_mbc3(uint8_t type)
{
    return type >= 0x0F && type <= 0x13;
}

static bool is_mbc5(uint8_t type)
{
    return type >= 0x19 && type <= 0x1E;
}

static bool is_mapped_mbc(uint8_t type)
{
    return is_mbc1(type) || is_mbc2(type) || is_mbc3(type) || is_mbc5(type);
}

static bool mbc5_has_rumble(uint8_t type)
{
    return type >= 0x1C && type <= 0x1E;
}

static uint16_t num_rom_banks(const cartridge *cart)
{
    uint16_t num_banks = (uint16_t)(cart->rom_size / ROM_BANK_SIZE);
    return num_banks > 0 ? num_banks : 1;
}

static uint16_t num_ram_banks(const cartridge *cart)
{
    uint16_t num_banks = (uint16_t)(cart->ram_size / EXT_RAM_BANK_SIZE);
    if (num_banks > 1) {
        return num_banks;
    }
    return num_banks == 0 ? 0 : 1;
}

static void set_ram_enable(cartridge *cart, uint8_t value)
{
    cart->ram_enable = (value == 0x0A) ? 0x0A : 0x00;
}

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
/*  MBC2 helpers                                                      */
/* ------------------------------------------------------------------ */

static void mbc2_recalculate_banks(cartridge *cart)
{
    uint8_t bank = cart->bank_reg_1 & 0x0F;
    if (bank == 0) {
        bank = 1;
    }
    cart->current_rom_bank = bank % num_rom_banks(cart);
}

static void mbc2_write_register(cartridge *cart, uint16_t addr, uint8_t value)
{
    if ((addr & 0x0100) == 0) {
        /* Bit 8 clear: RAM enable (lower 4 bits == 0xA) */
        cart->ram_enable = ((value & 0x0F) == 0x0A) ? 0x0A : 0x00;
        return;
    }

    /* Bit 8 set: ROM bank number (lower 4 bits, 0 maps to 1) */
    cart->bank_reg_1 = value & 0x0F;
    mbc2_recalculate_banks(cart);
}

static uint8_t mbc2_read_ram(cartridge *cart, uint16_t addr)
{
    if (cart->ram_enable != 0x0A || cart->ram == NULL) {
        return 0xFF;
    }

    size_t idx = addr & 0x1FF;
    if (idx >= cart->ram_size) {
        return 0xFF;
    }

    /* Only lower 4 bits are defined; upper bits read as 1 */
    return (uint8_t)(0xF0 | (cart->ram[idx] & 0x0F));
}

static void mbc2_write_ram(cartridge *cart, uint16_t addr, uint8_t value)
{
    if (cart->ram_enable != 0x0A || cart->ram == NULL) {
        return;
    }

    size_t idx = addr & 0x1FF;
    if (idx >= cart->ram_size) {
        return;
    }

    cart->ram[idx] = value & 0x0F;
}

/* ------------------------------------------------------------------ */
/*  MBC3 helpers                                                      */
/* ------------------------------------------------------------------ */

static bool mbc3_rtc_selected(const cartridge *cart)
{
    return cart->bank_reg_2 >= 0x08 && cart->bank_reg_2 <= 0x0C;
}

static uint8_t mbc3_rtc_index(const cartridge *cart)
{
    return (uint8_t)(cart->bank_reg_2 - 0x08);
}

static void mbc3_latch_rtc(cartridge *cart)
{
    for (uint8_t i = 0; i < MBC3_RTC_REG_COUNT; i++) {
        cart->rtc_latched[i] = cart->rtc_regs[i];
    }
}

static void mbc3_recalculate_banks(cartridge *cart)
{
    uint8_t bank = cart->bank_reg_1 & 0x7F;
    if (bank == 0) {
        bank = 1;
    }
    cart->current_rom_bank = bank % num_rom_banks(cart);

    if (mbc3_rtc_selected(cart)) {
        return;
    }

    uint8_t ram_bank = cart->bank_reg_2 & 0x07;
    uint8_t banks = num_ram_banks(cart);
    if (banks > 1) {
        ram_bank = (uint8_t)(ram_bank % banks);
    } else if (banks == 0) {
        ram_bank = 0;
    }
    cart->current_ram_bank = ram_bank;
}

static void mbc3_write_register(cartridge *cart, uint16_t addr, uint8_t value)
{
    if (addr <= MBC_RAM_ENABLE_END) {
        set_ram_enable(cart, value);
        return;
    }

    if (addr >= MBC_ROM_BANK_START && addr <= MBC_ROM_BANK_END) {
        cart->bank_reg_1 = value & 0x7F;
        mbc3_recalculate_banks(cart);
        return;
    }

    if (addr >= MBC_RAM_BANK_START && addr <= MBC_RAM_BANK_END) {
        cart->bank_reg_2 = value;
        mbc3_recalculate_banks(cart);
        return;
    }

    if (addr >= MBC_LATCH_CLOCK_START && addr <= MBC_LATCH_CLOCK_END) {
        if (value == 0x00) {
            cart->rtc_latch_ready = 1;
        } else if (value == 0x01 && cart->rtc_latch_ready) {
            mbc3_latch_rtc(cart);
            cart->rtc_latch_ready = 0;
        }
        return;
    }
}

static uint8_t mbc3_read_ext(cartridge *cart, uint16_t addr)
{
    if (mbc3_rtc_selected(cart)) {
        return cart->rtc_latched[mbc3_rtc_index(cart)];
    }

    if (cart->ram_enable != 0x0A || cart->ram == NULL) {
        return 0xFF;
    }

    size_t bank_offset = (size_t)cart->current_ram_bank * EXT_RAM_BANK_SIZE;
    size_t offset = bank_offset + (addr - EXTERNAL_RAM_START);
    if (offset < cart->ram_size) {
        return cart->ram[offset];
    }
    return 0xFF;
}

static void mbc3_write_ext(cartridge *cart, uint16_t addr, uint8_t value)
{
    if (mbc3_rtc_selected(cart)) {
        cart->rtc_regs[mbc3_rtc_index(cart)] = value;
        return;
    }

    if (cart->ram_enable != 0x0A || cart->ram == NULL) {
        return;
    }

    size_t bank_offset = (size_t)cart->current_ram_bank * EXT_RAM_BANK_SIZE;
    size_t offset = bank_offset + (addr - EXTERNAL_RAM_START);
    if (offset < cart->ram_size) {
        cart->ram[offset] = value;
    }
}

/* ------------------------------------------------------------------ */
/*  MBC5 helpers                                                      */
/* ------------------------------------------------------------------ */

static void mbc5_recalculate_banks(cartridge *cart)
{
    uint16_t bank = (uint16_t)(((cart->rom_bank_hi & 0x01) << 8) | cart->bank_reg_1);
    cart->current_rom_bank = (uint16_t)(bank % num_rom_banks(cart));

    uint8_t ram_bank = cart->bank_reg_2;
    if (mbc5_has_rumble(cart->mbc_type)) {
        ram_bank &= 0x07;
    } else {
        ram_bank &= 0x0F;
    }

    uint8_t banks = num_ram_banks(cart);
    if (banks > 1) {
        ram_bank = (uint8_t)(ram_bank % banks);
    } else if (banks == 0) {
        ram_bank = 0;
    }
    cart->current_ram_bank = ram_bank;
}

static void mbc5_write_register(cartridge *cart, uint16_t addr, uint8_t value)
{
    if (addr <= MBC_RAM_ENABLE_END) {
        set_ram_enable(cart, value);
        return;
    }

    if (addr >= MBC_ROM_BANK_START && addr <= 0x2FFF) {
        cart->bank_reg_1 = value;
        mbc5_recalculate_banks(cart);
        return;
    }

    if (addr >= MBC5_ROM_BANK_HI_START && addr <= MBC5_ROM_BANK_HI_END) {
        cart->rom_bank_hi = value & 0x01;
        mbc5_recalculate_banks(cart);
        return;
    }

    if (addr >= MBC_RAM_BANK_START && addr <= MBC_RAM_BANK_END) {
        cart->bank_reg_2 = value & 0x0F;
        mbc5_recalculate_banks(cart);
        return;
    }
}

static uint8_t mbc5_read_ext(cartridge *cart, uint16_t addr)
{
    if (cart->ram_enable != 0x0A || cart->ram == NULL) {
        return 0xFF;
    }

    size_t bank_offset = (size_t)cart->current_ram_bank * EXT_RAM_BANK_SIZE;
    size_t offset = bank_offset + (addr - EXTERNAL_RAM_START);
    if (offset < cart->ram_size) {
        return cart->ram[offset];
    }
    return 0xFF;
}

static void mbc5_write_ext(cartridge *cart, uint16_t addr, uint8_t value)
{
    if (cart->ram_enable != 0x0A || cart->ram == NULL) {
        return;
    }

    size_t bank_offset = (size_t)cart->current_ram_bank * EXT_RAM_BANK_SIZE;
    size_t offset = bank_offset + (addr - EXTERNAL_RAM_START);
    if (offset < cart->ram_size) {
        cart->ram[offset] = value;
    }
}

static uint8_t read_cartridge_rom(cartridge *cart, uint16_t addr)
{
    if (addr < ROM_BANK_ONE_START) {
        size_t offset = (size_t)addr;
        if (offset < cart->rom_size) {
            return cart->rom[offset];
        }
        return 0xFF;
    }

    size_t offset = (size_t)cart->current_rom_bank * ROM_BANK_SIZE +
                    (uint32_t)(addr - ROM_BANK_ONE_START);
    if (offset < cart->rom_size) {
        return cart->rom[offset];
    }
    return 0xFF;
}

static uint8_t read_mbc1_rom_bank0(cartridge *cart, uint16_t addr)
{
    uint8_t bank = mbc1_rom_bank_0(cart);
    size_t offset = (size_t)bank * ROM_BANK_SIZE + (uint32_t)addr;
    if (offset < cart->rom_size) {
        return cart->rom[offset];
    }
    return 0xFF;
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
        debug_assert(cartridge->mbc_type == 0 || is_mapped_mbc(cartridge->mbc_type));
    }

    /* Parse RAM size from cartridge header and allocate RAM */
    if (is_mbc2(cartridge->mbc_type)) {
        /* MBC2 has 512 x 4-bit built-in RAM regardless of header byte */
        cartridge->ram_size = MBC2_RAM_SIZE;
    } else if (cartridge->rom_size > RAM_SIZE_ADDR) {
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

    /* Recalculate MBC bank numbers based on initialized register values. */
    if (is_mbc1(mem->cartridge.mbc_type)) {
        mbc1_recalculate_banks(&mem->cartridge);
    } else if (is_mbc2(mem->cartridge.mbc_type)) {
        mbc2_recalculate_banks(&mem->cartridge);
    } else if (is_mbc3(mem->cartridge.mbc_type)) {
        mbc3_recalculate_banks(&mem->cartridge);
    } else if (is_mbc5(mem->cartridge.mbc_type)) {
        mbc5_recalculate_banks(&mem->cartridge);
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

        if (is_mbc1(cart->mbc_type)) {
            if (addr < ROM_BANK_ONE_START) {
                return read_mbc1_rom_bank0(cart, addr);
            }

            if (addr < VIDEO_RAM_START) {
                return read_cartridge_rom(cart, addr);
            }

            if (cart->ram_enable == 0x0A && cart->ram != NULL) {
                size_t bank_offset = (size_t)cart->current_ram_bank * EXT_RAM_BANK_SIZE;
                size_t offset = bank_offset + (addr - EXTERNAL_RAM_START);
                if (offset < cart->ram_size) {
                    return cart->ram[offset];
                }
            }
            return 0xFF;
        }

        if (is_mbc2(cart->mbc_type) || is_mbc3(cart->mbc_type) || is_mbc5(cart->mbc_type)) {
            if (addr < VIDEO_RAM_START) {
                return read_cartridge_rom(cart, addr);
            }

            if (is_mbc2(cart->mbc_type)) {
                return mbc2_read_ram(cart, addr);
            }
            if (is_mbc3(cart->mbc_type)) {
                return mbc3_read_ext(cart, addr);
            }
            return mbc5_read_ext(cart, addr);
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

        if (is_mbc1(cart->mbc_type)) {
            if (addr < VIDEO_RAM_START) {
                mbc1_write_register(mem, addr, value);
                return;
            }

            if (cart->ram_enable == 0x0A && cart->ram != NULL) {
                size_t bank_offset = (size_t)cart->current_ram_bank * EXT_RAM_BANK_SIZE;
                size_t offset = bank_offset + (addr - EXTERNAL_RAM_START);
                if (offset < cart->ram_size) {
                    cart->ram[offset] = value;
                }
            }
            return;
        }

        if (is_mbc2(cart->mbc_type)) {
            if (addr < VIDEO_RAM_START) {
                mbc2_write_register(cart, addr, value);
                return;
            }

            mbc2_write_ram(cart, addr, value);
            return;
        }

        if (is_mbc3(cart->mbc_type)) {
            if (addr < VIDEO_RAM_START) {
                mbc3_write_register(cart, addr, value);
                return;
            }

            mbc3_write_ext(cart, addr, value);
            return;
        }

        if (is_mbc5(cart->mbc_type)) {
            if (addr < VIDEO_RAM_START) {
                mbc5_write_register(cart, addr, value);
                return;
            }

            mbc5_write_ext(cart, addr, value);
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
        /* Bank switching makes pointer-based access unreliable for mapped MBCs. */
        cartridge *cart = &mem->cartridge;
        if (is_mapped_mbc(cart->mbc_type)) {
            debug_assert(0 && "access_memory8 on mapped MBC cartridge address not supported");
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