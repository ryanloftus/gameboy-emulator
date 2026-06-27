#include "ppu.h"

#define NUM_SCANLINES 154

#define LCD_CONTROL_REG_ADDR 0xFF40
#define LY_REG_ADDR 0xFF44
#define LYC_REG_ADDR 0xFF45
#define STAT_REG_ADDR 0xFF41
#define BGP_REG_ADDR 0xFF47

#define OBJ_PALETTE_0_ADDR 0xFF48
#define OBJ_PALETTE_1_ADDR 0xFF49

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
    if (ly >= 144) {
        ppu->mem->raw[STAT_REG_ADDR] |= 0b01; // VBlank - Mode 1
    }
}

uint8_t *bg_tile_map_start(memory *mem)
{
    uint8_t lcd_control = mem->raw[LCD_CONTROL_REG_ADDR];
    uint8_t bg_tile_map = (lcd_control >> 3) & 1;
    if (bg_tile_map) {
        return &(mem->raw[0x9C00]);
    }
    return &(mem->raw[0x9800]);
}

uint8_t *window_tile_map_start(memory *mem)
{
    uint8_t lcd_control = mem->raw[LCD_CONTROL_REG_ADDR];
    uint8_t window_tile_map = (lcd_control >> 6) & 1;
    if (window_tile_map) {
        return &(mem->raw[0x9C00]);
    }
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

void render_background(ppu *ppu, uint8_t scanline)
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
    
    uint8_t bgp = ppu->mem->raw[BGP_REG_ADDR];
    
    uint8_t *tile_map = bg_tile_map_start(ppu->mem);

    for (uint8_t col = 0; col < WIDTH; ++col)
    {
        uint16_t mapy = (scanline + yoffset) % 256;
        uint16_t mapx = (col + xoffset) % 256;
        uint8_t tile_id = tile_map[(mapy / 8) * 32 + (mapx / 8)];
        tile *t = get_tile(ppu->tile_cache, addressing_mode, tile_id);
        ppu->frame_buffer[scanline * WIDTH + col] = get_color(t->pixel[mapy % 8][mapx % 8], bgp);
    }
}

void render_window(ppu *ppu, uint8_t scanline)
{
    uint8_t lcd_control = ppu->mem->raw[LCD_CONTROL_REG_ADDR];
    uint8_t lcd_enabled = (lcd_control >> 7) & 1;
    uint8_t window_enabled = (lcd_control >> 5) & 1;
    uint8_t addressing_mode = (lcd_control >> 4) & 1;
    uint8_t window_x = ppu->mem->raw[0xFF4B];
    uint8_t window_y = ppu->mem->raw[0xFF4A];
    uint8_t bgp = ppu->mem->raw[BGP_REG_ADDR];

    if (!lcd_enabled) {
        return;
    }

    if (!window_enabled) {
        return;
    }

    uint8_t *window_map = window_tile_map_start(ppu->mem);
    for (uint8_t col = 0; col < WIDTH; ++col) {
        uint16_t mapy = (scanline + window_y) % 256;
        uint16_t mapx = (col + window_x - 7) % 256;
        uint8_t tile_id = window_map[(mapy / 8) * 32 + (mapx / 8)];
        tile *t = get_tile(ppu->tile_cache, addressing_mode, tile_id);
        ppu->frame_buffer[window_y * WIDTH + col] = get_color(t->pixel[mapy % 8][mapx % 8], bgp);
    }
}

void render_sprites(ppu *ppu, uint8_t scanline)
{
    uint8_t lcd_control = ppu->mem->raw[LCD_CONTROL_REG_ADDR];
    uint8_t lcd_enabled = (lcd_control >> 7) & 1;
    uint8_t obj_enabled = (lcd_control >> 1) & 1;
    uint8_t obj_size = (lcd_control >> 2) & 1;

    if (!lcd_enabled) {
        return;
    }

    if (!obj_enabled) {
        return;
    }

    uint8_t addressing_mode = 1;
    uint8_t *oam = &(ppu->mem->raw[0xFE00]);
    for (int i = 0; i < 40 * 4; i += 4) {
        uint8_t y = oam[i] - 16;
        uint8_t x = oam[i + 1] - 8;
        uint8_t tile_id = oam[i + 2];
        uint8_t attributes = oam[i + 3];

        uint8_t priority = (attributes >> 7) & 1;
        uint8_t y_flip = (attributes >> 6) & 1;
        uint8_t x_flip = (attributes >> 5) & 1;
        uint8_t palette_id = (attributes >> 4) & 1;
        uint8_t palette = ppu->mem->raw[OBJ_PALETTE_0_ADDR + palette_id];

        if (y > scanline || y + 16 < scanline) {
            continue;
        }

        tile *t = get_tile(ppu->tile_cache, addressing_mode, tile_id);
        for (int tile_offset_y = 0; tile_offset_y < 8; ++tile_offset_y) {
            for (int tile_offset_x = 0; tile_offset_x < 8; ++tile_offset_x) {
                uint8_t fb_y = y + tile_offset_y;
                uint8_t fb_x = x + tile_offset_x;
                if (fb_y < 0 || fb_y >= HEIGHT || fb_x < 0 || fb_x >= WIDTH) {
                    continue;
                }
                if (priority && ppu->frame_buffer[fb_y * WIDTH + fb_x] != 0) { // TODO: Priority is incorrect, should use color index 0 check, not actual color
                    continue;
                }
                uint8_t tile_y = y_flip ? 7 - tile_offset_y : tile_offset_y;
                uint8_t tile_x = x_flip ? 7 - tile_offset_x : tile_offset_x;
                uint8_t tile_pixel = t->pixel[tile_y][tile_x];
                if (tile_pixel != 0) {
                    ppu->frame_buffer[fb_y * WIDTH + fb_x] = get_color(tile_pixel, palette);
                }
            }
        }
        if (obj_size) {
            t = get_tile(ppu->tile_cache, addressing_mode, tile_id + 1);
            for (int tile_offset_y = 0; tile_offset_y < 8; ++tile_offset_y) {
                for (int tile_offset_x = 0; tile_offset_x < 8; ++tile_offset_x) {
                    uint8_t fb_y = y + 8 + tile_offset_y;
                    uint8_t fb_x = x - tile_offset_x;
                    if (fb_y < 0 || fb_y >= HEIGHT || fb_x < 0 || fb_x >= WIDTH) {
                        continue;
                    }
                    if (priority && ppu->frame_buffer[fb_y * WIDTH + fb_x] != 0) {
                        continue;
                    }
                    uint8_t tile_y = y_flip ? 7 - tile_offset_y : tile_offset_y;
                    uint8_t tile_x = x_flip ? 7 - tile_offset_x : tile_offset_x;
                    uint8_t tile_pixel = t->pixel[tile_y][tile_x];
                    if (tile_pixel != 0) {
                        ppu->frame_buffer[fb_y * WIDTH + fb_x] = get_color(tile_pixel, palette);
                    }
                }
            }
        }
    }
}

void render(ppu *ppu)
{
    uint8_t scanline = ppu->mem->raw[LY_REG_ADDR];
    if (scanline == 0) {
        populate_tiles(ppu);
    }

    if (scanline < 144) {
        render_background(ppu, scanline);
        render_window(ppu, scanline);
        render_sprites(ppu, scanline);
    }
    
    ppu->mem->raw[LY_REG_ADDR] = (scanline + 1) % NUM_SCANLINES;
    update_stat(ppu);
}

void init_ppu(ppu *ppu, memory *mem)
{
    ppu->mem = mem;
    memset(ppu->frame_buffer, 0, WIDTH * HEIGHT * sizeof(uint32_t));
    memset(ppu->tile_cache, 0, NUM_TILES * sizeof(tile));
}
