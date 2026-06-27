#ifndef _JOYPAD_H_
#define _JOYPAD_H_

#include "mmu.h"

#include <SDL.h>
#include <stdint.h>

/* P1 select bits (active low): bit 5 = buttons, bit 4 = d-pad. */
#define JOYP_SELECT_BUTTONS 0x10  /* JOYP_GET_BUTTONS — bit 5 clear */
#define JOYP_SELECT_DPAD    0x20  /* JOYP_GET_CTRL_PAD — bit 4 clear */
#define JOYP_SELECT_NONE    0x30  /* neither row selected */

/* Lower-nibble column positions (same bit index in d-pad and action rows). */
#define JOYP_RIGHT   0x01  /* bit 0: Right / A */
#define JOYP_LEFT    0x02  /* bit 1: Left / B */
#define JOYP_UP      0x04  /* bit 2: Up / Select */
#define JOYP_DOWN    0x08  /* bit 3: Down / Start */
#define JOYP_A       0x10
#define JOYP_B       0x20
#define JOYP_SELECT  0x40
#define JOYP_START   0x80

typedef struct {
    memory *mem;
} joypad;

void init_joypad(joypad *joypad, memory *mem);
void update_joypad(joypad *joypad, const SDL_Event *event);
void joypad_on_p1_write(memory *mem, uint8_t value);
uint8_t joypad_read_p1(const memory *mem);

#endif
