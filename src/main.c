#include "ppu.h"
#include "memory.h"
#include "cpu.h"

#include <SDL.h>

#define WIDTH 160
#define HEIGHT 144
#define SCALE 4

int main()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED, WIDTH*SCALE, HEIGHT*SCALE, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    uint32_t frame_buffer[WIDTH*HEIGHT];
    for (int y=0; y<HEIGHT; y++)
        for (int x=0; x<WIDTH; x++)
            frame_buffer[y*WIDTH + x] = (x ^ y) * 0x010101FF; // test pattern

    virtual_cpu cpu;
    create_virtual_cpu(&cpu);

    memory mem;
    init_memory(&mem);

    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
        }

        render(mem.video_ram, frame_buffer, WIDTH, HEIGHT);
        SDL_UpdateTexture(texture, NULL, frame_buffer, WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
