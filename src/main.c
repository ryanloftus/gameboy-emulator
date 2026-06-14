#include "ppu.h"
#include "mmu.h"
#include "cpu.h"
#include "debug.h"

#include <stdio.h>
#include <string.h>
#include <SDL.h>

#define WIDTH 160
#define HEIGHT 144
#define SCALE 4

/* One Game Boy frame is 154 scanlines × 456 T-cycles = 70224 T-cycles */
#define CYCLES_PER_FRAME 70224

int main(int argc, char *argv[])
{
    /* Parse flags before positional arguments */
    g_debug_mode = false;
    int rom_arg = 1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            g_debug_mode = true;
            rom_arg = i + 1;
        } else {
            rom_arg = i;
            break;
        }
    }

    if (rom_arg >= argc) {
        fprintf(stderr, "Usage: %s [--debug] <rom_file>\n", argv[0]);
        return 1;
    }

    const char *rom_path = argv[rom_arg];

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win = SDL_CreateWindow("ryanloftus/gameboy-emulator", SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED, WIDTH*SCALE, HEIGHT*SCALE, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    uint32_t frame_buffer[WIDTH*HEIGHT];
    for (int y=0; y<HEIGHT; y++)
        for (int x=0; x<WIDTH; x++)
            frame_buffer[y*WIDTH + x] = (x ^ y) * 0x010101FF; // test pattern

    memory mem;
    init_memory(&mem, rom_path);

    virtual_cpu cpu;
    create_virtual_cpu(&cpu, &mem);
    cpu.pc = 0x100;

    uint64_t next_vblank = CYCLES_PER_FRAME;

    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
        }

        /* Run CPU instructions until we reach the next VBlank boundary */
        while (cpu.cycles < next_vblank) {
            fetch_execute(&cpu);
        }
        next_vblank += CYCLES_PER_FRAME;

        /* Render the completed frame */
        render(&mem, frame_buffer, WIDTH, HEIGHT);

        /* Signal VBlank interrupt (IF bit 0) — occurs ~59.7 times a second */
        // mem.io_registers[(IF_REG_ADDR) & 0xFF] |= 0x01; // TODO: am i using vblank correctly?

        SDL_UpdateTexture(texture, NULL, frame_buffer, WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    destroy_memory(&mem);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
