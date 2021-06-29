#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "constants.h"
#include "cpu.h"
#include "gpu.h"
#include "joypad.h"
#include "memory.h"
#include "timer.h"

int quit(CPU* cpu, GPU* gpu, Memory* mem, Timer* timer, Joypad* joy, int returnCode);

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <path to ROM>\n", argv[0]);
        return 1;
    }

    Memory* mem = malloc(sizeof(*mem)); // freed in quit
    MEM_init(mem);
    MEM_loadROM(mem, argv[1]);

    printf("ROM info:\n");
    printf("Title: %s\n", mem->cartridge->title);
    printf("Cartridge type: 0x%02x\n", mem->cartridge->type);
    printf("ROM size: 0x%02x\n", mem->cartridge->romSize);
    printf("RAM size: 0x%02x\n", mem->cartridge->ramSize);

    CPU* cpu = malloc(sizeof(*cpu)); // freed in quit
    CPU_init(cpu);

    GPU* gpu = malloc(sizeof(*gpu)); // freed in quit
    GPU_init(gpu);

    Timer* timer = malloc(sizeof(*timer)); // freed in quit
    TIMER_init(timer);

    Joypad* joy = malloc(sizeof(*joy)); // freed in quit
    JOY_init(joy);

    #ifndef DISABLE_GRAPHICS
    // Init graphics
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow("yobeboy", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(renderer, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT);
    #endif

    SDL_Event event;
    uint32_t startTime = SDL_GetTicks();
    while (1) {
        int res = CPU_emulateCycle(cpu, gpu, mem, timer, joy);
        if (!res) {
            return quit(cpu, gpu, mem, timer, joy, 1);
        }
        GPU_update(cpu, gpu, mem);
        TIMER_update(cpu, mem, timer);
        MEM_dmaUpdate(mem);
        JOY_update(joy, mem);

        #ifndef DISABLE_GRAPHICS
        if (gpu->fbUpdated) {
            // Handle events
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT:
                        return quit(cpu, gpu, mem, timer, joy, 0);

                    case SDL_KEYDOWN:
                        switch (event.key.keysym.sym) {
                            case SDLK_a: joy->a = 1; break;
                            case SDLK_z: joy->b = 1; break;
                            case SDLK_RETURN: joy->start = 1; break;
                            case SDLK_BACKSPACE: joy->select = 1; break;
                            case SDLK_UP: joy->up = 1; break;
                            case SDLK_DOWN: joy->down = 1; break;
                            case SDLK_LEFT: joy->left = 1; break;
                            case SDLK_RIGHT: joy->right = 1; break;
                        }
                        break;

                    case SDL_KEYUP:
                        switch (event.key.keysym.sym) {
                            case SDLK_a: joy->a = 0; break;
                            case SDLK_z: joy->b = 0; break;
                            case SDLK_RETURN: joy->start = 0; break;
                            case SDLK_BACKSPACE: joy->select = 0; break;
                            case SDLK_UP: joy->up = 0; break;
                            case SDLK_DOWN: joy->down = 0; break;
                            case SDLK_LEFT: joy->left = 0; break;
                            case SDLK_RIGHT: joy->right = 0; break;
                        }
                        break;
                }
            }

            // Update the screen
            SDL_UpdateTexture(texture, NULL, gpu->framebuffer, GB_SCREEN_WIDTH * 4);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
            gpu->fbUpdated = false;

            // Maintain 60fps
            uint32_t elapsedTime = SDL_GetTicks() - startTime;
            uint32_t targetTime = 1000 / 60;
            if (elapsedTime <= targetTime) SDL_Delay(targetTime - elapsedTime);
            startTime = SDL_GetTicks();
        }
        #endif
    }
}

int quit(CPU* cpu, GPU* gpu, Memory* mem, Timer* timer, Joypad* joy, int returnCode) {
    // Dump external RAM to save
    if (mem->battery) {
        char* saveFileName = malloc(strlen(mem->romPath) + 5); // freed at the end of this block
        snprintf(saveFileName, strlen(mem->romPath) + 5, "%s.sav", mem->romPath);
        FILE* file = fopen(saveFileName, "wb+");
        for (int i = 0; i < (mem->extRamBanksNo * 0x2000); ++i) {
            fputc(mem->extRamBanks[i], file);
        }
        fclose(file);
        free(saveFileName);
    }

    // Destroy components
    CPU_destroy(cpu);
    GPU_destroy(gpu);
    MEM_destroy(mem);
    TIMER_destroy(timer);
    JOY_destroy(joy);

    return returnCode;
}