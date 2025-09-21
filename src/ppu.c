#include "ppu.h"

void render(uint8_t *vram, uint32_t *frame_buffer, uint8_t width, uint8_t height)
{
    for (int y=0; y<height; y++)
        for (int x=0; x<width; x++)
            frame_buffer[y*width + x] += 0xFF;
}
