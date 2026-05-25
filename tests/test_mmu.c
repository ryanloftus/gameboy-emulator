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
