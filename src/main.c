#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>

#include "constants.h"
#include "cpu.h"
#include "gpu.h"
#include "joypad.h"
#include "memory.h"
#include "timer.h"

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144

int quit(Memory* mem, int returnCode);

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <path to ROM>\n", argv[0]);
        return 1;
    }

    #ifndef DISABLE_GRAPHICS
    // Init graphics
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow("yobemag", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    #endif

    Memory mem;
    MEM_init(&mem);
    MEM_loadROM(&mem, argv[1]);

    CPU cpu;
    CPU_init(&cpu);

    GPU gpu;
    gpu.framebuffer = (uint8_t*) malloc(sizeof(uint8_t) * SCREEN_WIDTH * SCREEN_HEIGHT * 4);
    gpu.fbUpdated = 0;

    Timer timer;
    timer.divCounter = 0;

    Joypad joy;
    JOY_init(&joy);

    SDL_Event event;
    uint32_t startTime = SDL_GetTicks();
    while (1) {
        int res = CPU_emulateCycle(&cpu, &gpu, &mem, &timer, &joy);
        if (!res) {
            return quit(&mem, 1);
        }

        #ifndef DISABLE_GRAPHICS
        if (gpu.fbUpdated) {
            // Handle events
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT:
                        return quit(&mem, 0);

                    case SDL_KEYDOWN:
                        switch (event.key.keysym.sym) {
                            case SDLK_a: joy.a = 1; break;
                            case SDLK_z: joy.b = 1; break;
                            case SDLK_RETURN: joy.start = 1; break;
                            case SDLK_BACKSPACE: joy.select = 1; break;
                            case SDLK_UP: joy.up = 1; break;
                            case SDLK_DOWN: joy.down = 1; break;
                            case SDLK_LEFT: joy.left = 1; break;
                            case SDLK_RIGHT: joy.right = 1; break;
                        }
                        break;

                    case SDL_KEYUP:
                        switch (event.key.keysym.sym) {
                            case SDLK_a: joy.a = 0; break;
                            case SDLK_z: joy.b = 0; break;
                            case SDLK_RETURN: joy.start = 0; break;
                            case SDLK_BACKSPACE: joy.select = 0; break;
                            case SDLK_UP: joy.up = 0; break;
                            case SDLK_DOWN: joy.down = 0; break;
                            case SDLK_LEFT: joy.left = 0; break;
                            case SDLK_RIGHT: joy.right = 0; break;
                        }
                        break;
                }
            }

            // Update the screen
            SDL_UpdateTexture(texture, NULL, gpu.framebuffer, SCREEN_WIDTH * 4);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
            gpu.fbUpdated = 0;

            // Maintain 60fps
            uint32_t elapsedTime = SDL_GetTicks() - startTime;
            uint32_t targetTime = 1000 / 60;
            if (elapsedTime <= targetTime) SDL_Delay(targetTime - elapsedTime);
            startTime = SDL_GetTicks();
        }
        #endif
    }
}

int quit(Memory* mem, int returnCode) {
    if (mem->battery) {
        // Dump extram to save
        char saveFileName[128];
        sprintf(saveFileName, "%s.sav", mem->romPath);
        FILE* file = fopen(saveFileName, "wb+");
        for (int i = 0; i < (mem->extRamBanksNo * 0x2000); ++i) {
            fputc(mem->extRamBanks[i], file);
        }
        fclose(file);
    }
    return returnCode;
}