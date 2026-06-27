#ifndef _JOYPAD_H_
#define _JOYPAD_H_

#include "mmu.h"

#include <stdint.h>

typedef struct {
    memory *mem;
} joypad;

void init_joypad(joypad *joypad, memory *mem);
void update_joypad(joypad *joypad);

#endif