#ifndef _PPU_H_
#define _PPU_H_

#include <SDL.h>
#include <stdint.h>

/**
 * Populates frame_buffer from vram.
 */
void render(uint8_t *vram, uint32_t *frame_buffer, uint8_t width, uint8_t height);

#endif
