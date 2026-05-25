#include "unity.h"
#include "mmu.h"

#include <string.h>

static memory mem;

static void reset_mem(void)
{
    memset(&mem, 0, sizeof(mem));
}

void test_init_memory_zeroed(void)
{
    init_memory(&mem, NULL);
    for (int i = 0; i < 0x10000; i++) {
        TEST_ASSERT_EQUAL_UINT8(0, mem.raw[i]);
    }
}

void test_write_read_memory8(void)
{
    reset_mem();
    write_memory8(&mem, 0xC000, 0xAB);
    TEST_ASSERT_EQUAL_UINT8(0xAB, read_memory8(&mem, 0xC000));
}

void test_write_read_memory16(void)
{
    reset_mem();
    write_memory16(&mem, 0xC000, 0x1234);
    TEST_ASSERT_EQUAL_UINT16(0x1234, read_memory16(&mem, 0xC000));
    /* little-endian: low byte at addr, high byte at addr+1 */
    TEST_ASSERT_EQUAL_UINT8(0x34, mem.raw[0xC000]);
    TEST_ASSERT_EQUAL_UINT8(0x12, mem.raw[0xC001]);
}

void test_echo_ram_write_redirect(void)
{
    reset_mem();
    /* write to echo RAM at 0xE000 */
    write_memory8(&mem, 0xE000, 0x42);
    /* should be readable from work RAM at 0xC000 */
    TEST_ASSERT_EQUAL_UINT8(0x42, read_memory8(&mem, 0xC000));
}

void test_echo_ram_read_redirect(void)
{
    reset_mem();
    /* write to work RAM at 0xC000 */
    write_memory8(&mem, 0xC000, 0x99);
    /* should be readable from echo RAM at 0xE000 */
    TEST_ASSERT_EQUAL_UINT8(0x99, read_memory8(&mem, 0xE000));
}

void test_echo_ram_bidirectional(void)
{
    reset_mem();
    /* work → echo and echo → work should match */
    write_memory8(&mem, 0xC001, 0x77);
    TEST_ASSERT_EQUAL_UINT8(0x77, read_memory8(&mem, 0xE001));

    write_memory8(&mem, 0xE002, 0x33);
    TEST_ASSERT_EQUAL_UINT8(0x33, read_memory8(&mem, 0xC002));
}

void test_echo_ram_full_range_start(void)
{
    reset_mem();
    write_memory8(&mem, 0xC000, 0xAA);
    TEST_ASSERT_EQUAL_UINT8(0xAA, read_memory8(&mem, 0xE000));
}

void test_echo_ram_full_range_end(void)
{
    reset_mem();
    write_memory8(&mem, 0xDDFF, 0xBB);
    TEST_ASSERT_EQUAL_UINT8(0xBB, read_memory8(&mem, 0xFDFF));
}

void test_non_echo_not_affected(void)
{
    reset_mem();
    /* just above echo: 0xFE00 should NOT mirror */
    write_memory8(&mem, 0xC000, 0x12);
    TEST_ASSERT_EQUAL_UINT8(0x00, read_memory8(&mem, 0xFE00));
}

void test_work_ram_independent_from_echo(void)
{
    reset_mem();
    /* writing to work RAM should appear in echo, but not vice versa
       for the raw storage (they are the same location after redirect) */
    write_memory8(&mem, 0xC010, 0x55);
    TEST_ASSERT_EQUAL_UINT8(0x55, mem.raw[0xC010]);
    TEST_ASSERT_EQUAL_UINT8(0x00, mem.raw[0xE010]); /* raw not redirected */
}

/* ------------------------------------------------------------------ */
/*  MBC1 tests                                                        */
/* ------------------------------------------------------------------ */

/**
 * Helper: set up mem as an MBC1 cartridge with the given ROM size (in
 * bytes) and RAM size.  Fills ROM with sequential bytes so bank identity
 * can be verified.
 *
 * rom_size must be a multiple of ROM_BANK_SIZE.
 */
static void setup_mbc1_cartridge(memory *mem, size_t rom_size, size_t ram_size)
{
    memset(mem, 0, sizeof(*mem));

    mem->cartridge.mbc_type = 1;
    mem->cartridge.rom_size = rom_size;
    mem->cartridge.rom = calloc(1, rom_size);
    TEST_ASSERT_NOT_NULL(mem->cartridge.rom);

    /* Fill ROM so each byte carries bank * ROM_BANK_SIZE + offset mod 256 */
    for (size_t b = 0; b < rom_size / ROM_BANK_SIZE; b++) {
        for (size_t o = 0; o < ROM_BANK_SIZE; o++) {
            mem->cartridge.rom[b * ROM_BANK_SIZE + o] =
                (uint8_t)((b * ROM_BANK_SIZE + o) & 0xFF);
        }
    }

    /* MBC1 registers are zeroed by memset, which means:
     *   ram_enable   = 0  (disabled)
     *   bank_reg_1   = 0  (mapped to bank 1)
     *   bank_reg_2   = 0
     *   banking_mode = 0  (ROM mode)
     *   current_rom_bank = 1
     *   current_ram_bank = 0
     */

    if (ram_size > 0) {
        mem->cartridge.ram_size = ram_size;
        mem->cartridge.ram = calloc(1, ram_size);
        TEST_ASSERT_NOT_NULL(mem->cartridge.ram);
    }
}

void test_mbc1_rom_bank_0_default_rom_mode(void)
{
    setup_mbc1_cartridge(&mem, 2 * ROM_BANK_SIZE, 0);
    /* In ROM mode, bank 0 reads always come from physical bank 0 */
    uint8_t val = read_memory8(&mem, 0x0000);
    TEST_ASSERT_EQUAL_UINT8(0x00, val); /* offset 0 of bank 0 */
    val = read_memory8(&mem, 0x3FFF);
    TEST_ASSERT_EQUAL_UINT8(0xFF, val); /* offset 0x3FFF of bank 0 */
}

void test_mbc1_rom_bank_1_default(void)
{
    setup_mbc1_cartridge(&mem, 2 * ROM_BANK_SIZE, 0);
    /* With bank_reg_1 = 0, reads from 0x4000-0x7FFF should come from bank 1 */
    uint8_t val = read_memory8(&mem, 0x4000);
    /* bank 1, offset 0 → (1 * 0x4000 + 0) & 0xFF = 0x00 */
    TEST_ASSERT_EQUAL_UINT8(0x00, val);
    val = read_memory8(&mem, 0x7FFF);
    /* bank 1, offset 0x3FFF → (1 * 0x4000 + 0x3FFF) & 0xFF = 0xFF */
    TEST_ASSERT_EQUAL_UINT8(0xFF, val);
}

void test_mbc1_select_rom_bank_1(void)
{
    setup_mbc1_cartridge(&mem, 4 * ROM_BANK_SIZE, 0);
    /* Select bank 2 via writes to 0x2000-0x3FFF */
    write_memory8(&mem, 0x2100, 0x02);
    uint8_t val = read_memory8(&mem, 0x4000);
    /* bank 2, offset 0 → (2 * 0x4000 + 0) & 0xFF = 0x00 */
    TEST_ASSERT_EQUAL_UINT8(0x00, val);
    val = read_memory8(&mem, 0x7FFF);
    /* bank 2, offset 0x3FFF → (2 * 0x4000 + 0x3FFF) & 0xFF = 0xFF */
    TEST_ASSERT_EQUAL_UINT8(0xFF, val);
}

void test_mbc1_bank_1_wraps_zero_to_one(void)
{
    setup_mbc1_cartridge(&mem, 4 * ROM_BANK_SIZE, 0);
    /* Write 0 to bank_reg_1 (should map to bank 1) */
    write_memory8(&mem, 0x2100, 0x00);
    uint8_t val = read_memory8(&mem, 0x4000);
    /* bank 1, offset 0 → 0 */
    TEST_ASSERT_EQUAL_UINT8(0x00, val);
}

void test_mbc1_rom_mode_upper_bits(void)
{
    setup_mbc1_cartridge(&mem, 128 * ROM_BANK_SIZE, 0); /* 2 MB ROM */
    /* In ROM mode (banking_mode = 0), bank_reg_2 provides upper bits.
     * Select bank_reg_1 = 1 (lower 5 bits) + bank_reg_2 = 1 (upper 2 bits) = bank 33 */
    write_memory8(&mem, 0x2100, 0x01);   /* bank_reg_1 = 1 */
    write_memory8(&mem, 0x4100, 0x01);   /* bank_reg_2 = 1; ROM mode */
    /* current_rom_bank should be (1 << 5) | 1 = 33 */
    TEST_ASSERT_EQUAL_UINT8(33, mem.cartridge.current_rom_bank);
    uint8_t val = read_memory8(&mem, 0x4000);
    /* bank 33, offset 0 → (33 * 0x4000 + 0) & 0xFF = 0x00 */
    TEST_ASSERT_EQUAL_UINT8(0x00, val);
}

void test_mbc1_ram_enable_disable(void)
{
    setup_mbc1_cartridge(&mem, 2 * ROM_BANK_SIZE, EXT_RAM_BANK_SIZE);

    /* RAM starts disabled (ram_enable = 0). Try writing to external RAM */
    write_memory8(&mem, 0xA000, 0x55);
    uint8_t val = read_memory8(&mem, 0xA000);
    TEST_ASSERT_EQUAL_UINT8(0xFF, val); /* disabled → reads return 0xFF */

    /* Enable RAM by writing 0x0A to 0x0000-0x1FFF */
    write_memory8(&mem, 0x0000, 0x0A);
    TEST_ASSERT_EQUAL_UINT8(0x0A, mem.cartridge.ram_enable);

    write_memory8(&mem, 0xA000, 0x55);
    val = read_memory8(&mem, 0xA000);
    TEST_ASSERT_EQUAL_UINT8(0x55, val);

    /* Disable RAM by writing any value other than 0x0A */
    write_memory8(&mem, 0x0000, 0x00);
    TEST_ASSERT_EQUAL_UINT8(0x00, mem.cartridge.ram_enable);

    write_memory8(&mem, 0xA001, 0xAA);
    val = read_memory8(&mem, 0xA001);
    TEST_ASSERT_EQUAL_UINT8(0xFF, val); /* still disabled */
}

void test_mbc1_external_ram_rom_mode_bank_0(void)
{
    setup_mbc1_cartridge(&mem, 2 * ROM_BANK_SIZE, 4 * EXT_RAM_BANK_SIZE);
    /* Enable RAM */
    write_memory8(&mem, 0x0000, 0x0A);

    /* In ROM mode, RAM bank should always be 0 regardless of bank_reg_2 */
    write_memory8(&mem, 0x4100, 0x02); /* set bank_reg_2 = 2 */
    TEST_ASSERT_EQUAL_UINT8(0, mem.cartridge.current_ram_bank);

    write_memory8(&mem, 0xA000, 0x77);
    TEST_ASSERT_EQUAL_UINT8(0x77, read_memory8(&mem, 0xA000));

    /* Verify bank 0 storage in cartridge RAM */
    TEST_ASSERT_EQUAL_UINT8(0x77, mem.cartridge.ram[0]);
}

void test_mbc1_external_ram_mode_switch_bank(void)
{
    setup_mbc1_cartridge(&mem, 2 * ROM_BANK_SIZE, 4 * EXT_RAM_BANK_SIZE);
    /* Enable RAM */
    write_memory8(&mem, 0x0000, 0x0A);

    /* Switch to RAM mode */
    write_memory8(&mem, 0x6000, 0x01);
    TEST_ASSERT_EQUAL_UINT8(1, mem.cartridge.banking_mode);

    /* Select RAM bank 2 via bank_reg_2 */
    write_memory8(&mem, 0x4100, 0x02);
    TEST_ASSERT_EQUAL_UINT8(2, mem.cartridge.current_ram_bank);

    /* Write to external RAM (should go to bank 2) */
    write_memory8(&mem, 0xA000, 0xBB);
    TEST_ASSERT_EQUAL_UINT8(0xBB, read_memory8(&mem, 0xA000));

    /* Verify bank 2 storage in cartridge RAM */
    TEST_ASSERT_EQUAL_UINT8(0x00, mem.cartridge.ram[0]);                              /* bank 0 unchanged */
    TEST_ASSERT_EQUAL_UINT8(0xBB, mem.cartridge.ram[2 * EXT_RAM_BANK_SIZE]);          /* bank 2 written */
}

void test_mbc1_ram_mode_maps_rom_bank_0(void)
{
    setup_mbc1_cartridge(&mem, 128 * ROM_BANK_SIZE, EXT_RAM_BANK_SIZE);

    /* In ROM mode (default), bank 0 reads come from physical bank 0 */
    uint8_t val = read_memory8(&mem, 0x0000);
    TEST_ASSERT_EQUAL_UINT8(0x00, val);

    /* Switch to RAM mode with bank_reg_2 = 0 → bank 0 should still be visible */
    write_memory8(&mem, 0x6000, 0x01); /* RAM mode */
    write_memory8(&mem, 0x4100, 0x00); /* bank_reg_2 = 0 */
    val = read_memory8(&mem, 0x0000);
    TEST_ASSERT_EQUAL_UINT8(0x00, val); /* still bank 0 */

    /* Set bank_reg_2 = 1 in RAM mode → bank 0 window maps to bank (1 << 5) = 32 */
    write_memory8(&mem, 0x4100, 0x01);
    /* With bank_reg_2 = 1 and banking_mode = 1, bank_0 should be 32 */
    val = read_memory8(&mem, 0x0000);
    /* bank 32, offset 0 → (32 * 0x4000) & 0xFF = 0x00 */
    TEST_ASSERT_EQUAL_UINT8(0x00, val);
    /* Check offset 0x3FFF in bank 32 → (32 * 0x4000 + 0x3FFF) & 0xFF = 0xFF */
    val = read_memory8(&mem, 0x3FFF);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)(32 * ROM_BANK_SIZE + 0x3FFF) & 0xFF, val);
}

void test_mbc1_write_to_rom_does_not_alter_rom(void)
{
    setup_mbc1_cartridge(&mem, 2 * ROM_BANK_SIZE, 0);

    /* Read initial value */
    uint8_t orig = read_memory8(&mem, 0x0000);
    TEST_ASSERT_EQUAL_UINT8(0x00, orig);

    /* Try writing to ROM space (should be a no-op / redirected to MBC registers) */
    write_memory8(&mem, 0x0000, 0xFF);
    /* ROM should be unchanged */
    TEST_ASSERT_EQUAL_UINT8(orig, mem.cartridge.rom[0]);
}

void test_mbc1_switch_from_rom_to_ram_mode(void)
{
    setup_mbc1_cartridge(&mem, 128 * ROM_BANK_SIZE, 4 * EXT_RAM_BANK_SIZE);

    /* Start in ROM mode. Set bank_reg_1 = 1, bank_reg_2 = 1 */
    write_memory8(&mem, 0x2100, 0x01);
    write_memory8(&mem, 0x4100, 0x01);
    /* In ROM mode: current_rom_bank = (1 << 5) | 1 = 33, current_ram_bank = 0 */
    TEST_ASSERT_EQUAL_UINT8(33, mem.cartridge.current_rom_bank);
    TEST_ASSERT_EQUAL_UINT8(0, mem.cartridge.current_ram_bank);

    /* Switch to RAM mode */
    write_memory8(&mem, 0x6000, 0x01);
    /* In RAM mode: bank_reg_2 used for RAM bank, so current_rom_bank = 1 (only lower 5 bits) */
    TEST_ASSERT_EQUAL_UINT8(1, mem.cartridge.current_rom_bank);
    /* current_ram_bank = bank_reg_2 & 0x03 = 1 */
    TEST_ASSERT_EQUAL_UINT8(1, mem.cartridge.current_ram_bank);
}

void test_mbc1_mode_switch_affects_bank_0_window(void)
{
    setup_mbc1_cartridge(&mem, 128 * ROM_BANK_SIZE, 4 * EXT_RAM_BANK_SIZE);

    /* ROM mode: bank 0 window reads from physical bank 0 */
    write_memory8(&mem, 0x4100, 0x01); /* bank_reg_2 = 1 */
    uint8_t val = read_memory8(&mem, 0x0000);
    TEST_ASSERT_EQUAL_UINT8(0x00, val); /* bank 0, offset 0 */

    /* Switch to RAM mode with bank_reg_2 = 1 */
    write_memory8(&mem, 0x6000, 0x01);
    /* Now bank 0 window maps to bank (1 << 5) = 32 */
    val = read_memory8(&mem, 0x0000);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)(32 * ROM_BANK_SIZE) & 0xFF, val);
}

void test_mbc1_rom_addr_wraps_modulo_num_banks(void)
{
    /* With a small ROM (4 banks), selecting a bank beyond the count should wrap */
    setup_mbc1_cartridge(&mem, 4 * ROM_BANK_SIZE, 0);

    /* Select bank_reg_2 = 1, bank_reg_1 = 1 → (1 << 5) | 1 = 33, but only 4 banks exist */
    write_memory8(&mem, 0x2100, 0x01);
    write_memory8(&mem, 0x4100, 0x01);

    /* 33 % 4 = 1 */
    TEST_ASSERT_EQUAL_UINT8(1, mem.cartridge.current_rom_bank);

    uint8_t val = read_memory8(&mem, 0x4000);
    /* bank 1, offset 0 → (1 * 0x4000) & 0xFF = 0x00 */
    TEST_ASSERT_EQUAL_UINT8(0x00, val);
}

void test_mbc1_ram_not_allocated_returns_ff(void)
{
    /* Cartridge type 1 with no RAM */
    setup_mbc1_cartridge(&mem, 2 * ROM_BANK_SIZE, 0);

    write_memory8(&mem, 0x0000, 0x0A); /* enable RAM (even though none allocated) */
    uint8_t val = read_memory8(&mem, 0xA000);
    TEST_ASSERT_EQUAL_UINT8(0xFF, val);

    /* Write should be a no-op */
    write_memory8(&mem, 0xA000, 0x55);
    val = read_memory8(&mem, 0xA000);
    TEST_ASSERT_EQUAL_UINT8(0xFF, val);
}

void test_mbc1_ram_bank_wraps_modulo(void)
{
    setup_mbc1_cartridge(&mem, 2 * ROM_BANK_SIZE, 2 * EXT_RAM_BANK_SIZE); /* 2 RAM banks */
    write_memory8(&mem, 0x0000, 0x0A);

    /* Switch to RAM mode */
    write_memory8(&mem, 0x6000, 0x01);

    /* Select RAM bank 3 — but only 2 banks exist (0, 1), so 3 % 2 = 1 */
    write_memory8(&mem, 0x4100, 0x03);
    TEST_ASSERT_EQUAL_UINT8(1, mem.cartridge.current_ram_bank);

    write_memory8(&mem, 0xA000, 0xCC);
    TEST_ASSERT_EQUAL_UINT8(0xCC, read_memory8(&mem, 0xA000));
    TEST_ASSERT_EQUAL_UINT8(0xCC, mem.cartridge.ram[1 * EXT_RAM_BANK_SIZE]); /* bank 1 */
    TEST_ASSERT_EQUAL_UINT8(0x00, mem.cartridge.ram[0 * EXT_RAM_BANK_SIZE]); /* bank 0 unchanged */
}
