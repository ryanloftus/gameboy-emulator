#ifndef _JOYPAD_H_
#define _JOYPAD_H_

#include "mmu.h"

#include <SDL.h>
#include <stdint.h>

/* Bit layout matches P1 lower-nibble column positions (see Pan Docs). */
#define JOYP_DOWN    0x01
#define JOYP_UP      0x02
#define JOYP_LEFT    0x04
#define JOYP_RIGHT   0x08
#define JOYP_A       0x10
#define JOYP_B       0x20
#define JOYP_SELECT  0x40
#define JOYP_START   0x80

typedef struct {
    memory *mem;
} joypad;

void init_joypad(joypad *joypad, memory *mem);
void update_joypad(joypad *joypad, const SDL_Event *event);
uint8_t joypad_read_p1(const memory *mem);

#endif
