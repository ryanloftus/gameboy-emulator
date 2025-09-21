#include "ppu.h"

#define NUM_TILES 384

typedef struct 
{
    // each pixel should be in the range [0,3]
    uint8_t pixel[8][8];
} tile;

void populate_tiles(memory *mem, tile *tiles)
{
    for (int i = 0; i < NUM_TILES; ++i)
    {
        uint16_t tile_start = i * sizeof(tile);
        for (int row = 0; row < 8; ++row)
        {
            uint8_t b1 = mem->video_ram[tile_start + row * 2];
            uint8_t b2 = mem->video_ram[tile_start + row * 2 + 1];
            for (int col = 0; col < 8; ++col)
            {
                uint8_t lowbit = (b1 >> col) & 1;
                uint8_t highbit = (b2 >> col) & 1;
                tiles[i].pixel[row][col] = (highbit << 1) | lowbit;
            }
        }
    }
}

uint8_t *tile_map_start(memory *mem)
{
    return &(mem->raw[0x9800]);
}

tile *get_tile(tile *tiles, uint8_t addressing_mode, uint8_t id)
{
    if (addressing_mode)
    {
        return &(tiles[id]);
    }
    
    return &(tiles[128 + (int8_t)id]);
}

uint32_t get_color(uint8_t pixel)
{
    switch (pixel)
    {
        case 0: return 0;
        case 1: return 0x888888FF;
        case 2: return 0xC8C8C8FF;
        case 3: return 0xFFFFFFFF;
    }
    return 0xEE40FFFF;
}

void render_background(memory *mem, uint32_t *frame_buffer, uint8_t width, uint8_t height)
{
    tile tiles[NUM_TILES];
    populate_tiles(mem, &tiles);

    uint8_t yoffset = mem->raw[0xFF42];
    uint8_t xoffset = mem->raw[0xFF43];

    uint8_t addressing_mode = (mem->raw[0xFF40] >> 4) & 1;

    uint8_t *tile_map = tile_map_start(mem);

    for (uint8_t row = 0; row < height; ++row)
    {
        for (uint8_t col = 0; col < width; ++col)
        {
            uint16_t mapy = (row + yoffset) % 256;
            uint16_t mapx = (col + xoffset) % 256;
            uint8_t tile_id = tile_map[(mapy / 8) * 32 + (mapx / 8)];
            tile *t = get_tile(tiles, addressing_mode, tile_id);
            frame_buffer[row * width + col] = get_color(t->pixel[mapy % 8][mapx % 8]);
        }
    }
}

void render_window(memory *mem, uint32_t *frame_buffer, uint8_t width, uint8_t height)
{

}

void render_sprites(memory *mem, uint32_t *frame_buffer, uint8_t width, uint8_t height)
{

}

void render(memory *mem, uint32_t *frame_buffer, uint8_t width, uint8_t height)
{
    render_background(mem, frame_buffer, width, height);
    render_window(mem, frame_buffer, width, height);
    render_sprites(mem, frame_buffer, width, height);
}
