#ifndef _PPU_H_
#define _PPU_H_

#include "mmu.h"

#include <SDL.h>
#include <stdint.h>

#define WIDTH 160
#define HEIGHT 144

#define NUM_TILES 384

typedef struct
{
    // each pixel should be in the range [0,3]
    uint8_t pixel[8][8];
} tile;

typedef struct {
    memory *mem;
    uint32_t frame_buffer[WIDTH*HEIGHT];
    tile tile_cache[NUM_TILES];
} ppu;

/**
 * Initializes the PPU.
 */
void init_ppu(ppu *ppu, memory *mem);

/**
 * Performs one scanline of rendering.
 */
void render(ppu *ppu);

#endif
