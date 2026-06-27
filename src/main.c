#include "ppu.h"
#include "mmu.h"
#include "cpu.h"
#include "joypad.h"
#include "debug.h"

#include <stdio.h>
#include <string.h>
#include <SDL.h>

#define SCALE 4

/* One Game Boy frame is 154 scanlines × 456 T-cycles = 70224 T-cycles */
#define CYCLES_PER_DOT 4
#define DOTS_PER_SCANLINE 456
#define SCANLINES_PER_FRAME 154

int main(int argc, char *argv[])
{
    /* Parse flags before positional arguments */
    g_debug_mode = false;
    g_instr_log = false;
    int rom_arg = 1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            g_debug_mode = true;
        } else if (strcmp(argv[i], "--trace") == 0) {
            g_instr_log = true;
        } else {
            rom_arg = i;
            break;
        }
    }

    if (rom_arg >= argc) {
        fprintf(stderr, "Usage: %s [--debug] [--trace] <rom_file>\n", argv[0]);
        return 1;
    }

    const char *rom_path = argv[rom_arg];

    if (g_instr_log) {
        init_debug_log();
    }

    SDL_Window *win = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;

    if (!g_debug_mode) {
        SDL_Init(SDL_INIT_VIDEO);
        win = SDL_CreateWindow("ryanloftus/gameboy-emulator", SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED, WIDTH * SCALE, HEIGHT * SCALE, 0);
        renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                    SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    }

    memory mem;
    init_memory(&mem, rom_path);

    virtual_cpu cpu;
    create_virtual_cpu(&cpu, &mem);
    cpu.pc = 0x100;

    ppu ppu;
    init_ppu(&ppu, &mem);

    joypad joypad;
    init_joypad(&joypad, &mem);

    uint64_t next_render = 0;
    uint64_t next_vblank = 0;

    int running = 1;
    while (running) {
        if (!g_debug_mode) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) running = 0;
            }
        }

        /* Run CPU instructions until we reach the next VBlank boundary */
        while (cpu.cycles < next_render) {
            fetch_execute(&cpu);
        }

        next_render += CYCLES_PER_DOT * DOTS_PER_SCANLINE;
        render(&ppu);

        if (!g_debug_mode && cpu.cycles >= next_vblank) {
            SDL_UpdateTexture(texture, NULL, ppu.frame_buffer, WIDTH * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }
    }

    destroy_memory(&mem);

    if (g_instr_log) {
        close_debug_log();
    }

    if (!g_debug_mode) {
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        SDL_Quit();
    }

    return 0;
}
