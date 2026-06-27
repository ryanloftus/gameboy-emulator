#include "unity.h"
#include "joypad.h"
#include "mmu.h"

#include <string.h>

static memory mem;

static void reset_joypad(void)
{
    memset(&mem, 0, sizeof(mem));
    joypad jp;
    init_joypad(&jp, &mem);
}

void test_joypad_select_buttons_reads_action_row(void)
{
    reset_joypad();
    mem.joypad_buttons = JOYP_START;
    write_memory8(&mem, JOYPAD_REG_ADDR, JOYP_SELECT_BUTTONS);

    uint8_t p1 = read_memory8(&mem, JOYPAD_REG_ADDR);
    TEST_ASSERT_BITS(0x30, JOYP_SELECT_BUTTONS, p1);
    TEST_ASSERT_BITS(0x08, 0x00, p1); /* Start pressed → bit 3 clear */
}

void test_joypad_select_dpad_reads_direction_row(void)
{
    reset_joypad();
    mem.joypad_buttons = JOYP_DOWN;
    write_memory8(&mem, JOYPAD_REG_ADDR, JOYP_SELECT_DPAD);

    uint8_t p1 = read_memory8(&mem, JOYPAD_REG_ADDR);
    TEST_ASSERT_BITS(0x30, JOYP_SELECT_DPAD, p1);
    TEST_ASSERT_BITS(0x08, 0x00, p1); /* Down pressed → bit 3 clear */
}

void test_joypad_select_none_reads_all_released(void)
{
    reset_joypad();
    mem.joypad_buttons = JOYP_A | JOYP_UP;
    write_memory8(&mem, JOYPAD_REG_ADDR, JOYP_SELECT_NONE);

    uint8_t p1 = read_memory8(&mem, JOYPAD_REG_ADDR);
    TEST_ASSERT_EQUAL_UINT8(0xFF, p1);
}

void test_joypad_select_write_fires_if_on_held_button(void)
{
    reset_joypad();
    mem.joypad_buttons = JOYP_START;
    write_memory8(&mem, JOYPAD_REG_ADDR, JOYP_SELECT_NONE);
    mem.io_registers[IF_REG_ADDR & 0xFF] = 0;

    write_memory8(&mem, JOYPAD_REG_ADDR, JOYP_SELECT_BUTTONS);

    TEST_ASSERT_BITS(0x10, 0x10, mem.io_registers[IF_REG_ADDR & 0xFF]);
}
