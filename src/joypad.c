#include "joypad.h"

#define JOYPAD_REG_ADDR 0xFF00

void init_joypad(joypad *joypad, memory *mem)
{
    joypad->mem = mem;
    mem->raw[JOYPAD_REG_ADDR] = 0xCF;
}

void update_joypad(joypad *joypad)
{
    uint8_t joypad_reg = joypad->mem->raw[JOYPAD_REG_ADDR];
}
