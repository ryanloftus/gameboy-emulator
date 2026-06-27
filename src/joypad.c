#include "joypad.h"

void init_joypad(joypad *joypad, memory *mem)
{
    joypad->mem = mem;
    mem->raw[JOYPAD_REG_ADDR] = 0x30;
}

void update_joypad(joypad *joypad)
{
    (void)joypad;
}
