#include "ppu.h"
#include "mmu.h"
#include "cpu.h"
#include "joypad.h"
#include "debug.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <SDL.h>

#define SCALE 4

/* One Game Boy frame is 154 scanlines × 456 T-cycles = 70224 T-cycles.
 * cpu->cycles counts M-cycles (4 T-cycles each). */
#define DOTS_PER_SCANLINE 456
#define M_CYCLES_PER_SCANLINE (DOTS_PER_SCANLINE / 4)
#define SCANLINES_PER_FRAME 154
#define M_CYCLES_PER_FRAME (M_CYCLES_PER_SCANLINE * SCANLINES_PER_FRAME)

/* DMG system clock is 4.194304 MHz → 1.048576 M-cycles per second. */
#define GB_M_CYCLES_PER_SEC 1048576ULL
#define THROTTLE_INTERVAL   100ULL

static uint64_t monotonic_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static void sleep_ns(uint64_t ns)
{
    struct timespec req = {
        .tv_sec = (time_t)(ns / 1000000000ULL),
        .tv_nsec = (long)(ns % 1000000000ULL),
    };
    nanosleep(&req, NULL);
}

/* Keep emulated time aligned with real time, checking every THROTTLE_INTERVAL M-cycles. */
static void throttle_pace(uint64_t cpu_cycles, uint64_t *last_cycles, uint64_t *last_ns)
{
    uint64_t delta = cpu_cycles - *last_cycles;
    if (delta < THROTTLE_INTERVAL) {
        return;
    }

    uint64_t now = monotonic_ns();
    uint64_t expected_ns = delta * 1000000000ULL / GB_M_CYCLES_PER_SEC;
    uint64_t elapsed_ns = now - *last_ns;

    if (elapsed_ns < expected_ns) {
        sleep_ns(expected_ns - elapsed_ns);
    }

    *last_cycles = cpu_cycles;
    *last_ns = monotonic_ns();
}

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
    uint64_t next_vblank = M_CYCLES_PER_FRAME;
    uint64_t throttle_last_cycles = 0;
    uint64_t throttle_last_ns = monotonic_ns();

    int running = 1;
    while (running) {
        if (!g_debug_mode) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = 0;
                } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                    update_joypad(&joypad, &event);
                }
            }
        }

        /* Run CPU instructions until we reach the next scanline boundary */
        while (cpu.cycles < next_render) {
            fetch_execute(&cpu);
            if (!g_debug_mode) {
                throttle_pace(cpu.cycles, &throttle_last_cycles, &throttle_last_ns);
            }
        }

        next_render += M_CYCLES_PER_SCANLINE;
        render(&ppu);

        if (!g_debug_mode && cpu.cycles >= next_vblank) {
            SDL_UpdateTexture(texture, NULL, ppu.frame_buffer, WIDTH * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
            next_vblank += M_CYCLES_PER_FRAME;
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
