#include "ppu.h"

#define NUM_SCANLINES 154

#define LCD_CONTROL_REG_ADDR 0xFF40
#define LY_REG_ADDR 0xFF44
#define LYC_REG_ADDR 0xFF45
#define STAT_REG_ADDR 0xFF41
#define BGP_REG_ADDR 0xFF47

void populate_tiles(ppu *ppu)
{
    for (int i = 0; i < NUM_TILES; ++i)
    {
        uint16_t tile_start = i * 16;
        for (int row = 0; row < 8; ++row)
        {
            uint8_t b1 = ppu->mem->video_ram[tile_start + row * 2];
            uint8_t b2 = ppu->mem->video_ram[tile_start + row * 2 + 1];
            for (int col = 0; col < 8; ++col)
            {
                uint8_t lowbit = (b1 >> (7-col)) & 1;
                uint8_t highbit = (b2 >> (7-col)) & 1;
                ppu->tile_cache[i].pixel[row][col] = (highbit << 1) | lowbit;
            }
        }
    }
}

void update_stat(ppu *ppu)
{
    uint8_t ly = ppu->mem->raw[LY_REG_ADDR];
    uint8_t lyc = ppu->mem->raw[LYC_REG_ADDR];
    ppu->mem->raw[STAT_REG_ADDR] &= 0b11111000;
    ppu->mem->raw[STAT_REG_ADDR] |= (ly == lyc ? 0b100 : 0);
    if (ly == 144) {
        /* Signal VBlank interrupt (IF bit 0) — occurs ~59.7 times a second */
        ppu->mem->io_registers[(IF_REG_ADDR) & 0xFF] |= 0x01; // TODO: am i using vblank correctly?
    }
    
    // TODO: set mode bits
    if (ly > 144) {
        ppu->mem->raw[STAT_REG_ADDR] |= 0b01; // VBlank - Mode 1
    }
}

uint8_t *tile_map_start(memory *mem)
{
    return &(mem->raw[0x9800]);
}

tile *get_tile(tile *tiles, uint8_t addressing_mode, uint8_t id)
{
    if (id >= 128 || addressing_mode) {
        return &(tiles[id]);
    }

    return &(tiles[256 + (uint16_t)id]);
}

uint32_t get_color(uint8_t pixel, uint8_t bgp)
{
    uint8_t color_id = (bgp >> (pixel * 2)) & 0b11;
    switch (color_id)
    {
        case 0: return 0xFFFFFFFF;
        case 1: return 0xC8C8C8FF;
        case 2: return 0x888888FF;
        case 3: return 0;
    }
    return 0xEE40FFFF;
}

void render_background(ppu *ppu)
{
    uint8_t yoffset = ppu->mem->raw[0xFF42];
    uint8_t xoffset = ppu->mem->raw[0xFF43];

    uint8_t lcd_control = ppu->mem->raw[LCD_CONTROL_REG_ADDR];
    uint8_t lcd_enabled = (lcd_control >> 7) & 1;
    uint8_t addressing_mode = (lcd_control >> 4) & 1;
    
    if (!lcd_enabled) {
        memset(ppu->frame_buffer, 0, WIDTH * HEIGHT * sizeof(uint32_t));
        return;
    }
    
    uint8_t ly = ppu->mem->raw[LY_REG_ADDR];
    uint8_t bgp = ppu->mem->raw[BGP_REG_ADDR];
    
    if (ly == 0) {
        populate_tiles(ppu);
    }
    
    if (ly < 144) {
        uint8_t *tile_map = tile_map_start(ppu->mem);
    
        for (uint8_t col = 0; col < WIDTH; ++col)
        {
            uint16_t mapy = (ly + yoffset) % 256;
            uint16_t mapx = (col + xoffset) % 256;
            uint8_t tile_id = tile_map[(mapy / 8) * 32 + (mapx / 8)];
            tile *t = get_tile(ppu->tile_cache, addressing_mode, tile_id);
            ppu->frame_buffer[ly * WIDTH + col] = get_color(t->pixel[mapy % 8][mapx % 8], bgp);
        }
    }

    ppu->mem->raw[LY_REG_ADDR] = (ly + 1) % NUM_SCANLINES;
    update_stat(ppu);
}

void render_window(ppu *ppu)
{

}

void render_sprites(ppu *ppu)
{

}

void render(ppu *ppu)
{
    render_background(ppu);
    render_window(ppu);
    render_sprites(ppu);
}

void init_ppu(ppu *ppu, memory *mem)
{
    ppu->mem = mem;
    memset(ppu->frame_buffer, 0, WIDTH * HEIGHT * sizeof(uint32_t));
    memset(ppu->tile_cache, 0, NUM_TILES * sizeof(tile));
}
