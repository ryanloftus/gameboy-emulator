#ifndef _PPU_H_
#define _PPU_H_

#include "mmu.h"

#include <SDL.h>
#include <stdint.h>

/**
 * Populates frame_buffer from vram.
 */
void render(memory *mem, uint32_t *frame_buffer, uint8_t width, uint8_t height);

#endif
