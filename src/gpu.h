#ifndef GPU_H
#define GPU_H

#include <stdbool.h>

typedef struct GPU GPU;

#include "cpu.h"
#include "memory.h"

struct GPU {
    int machineCycleCounter;
    uint8_t* framebuffer;
    bool fbUpdated;

    uint8_t* backgroundMap;
    uint8_t* windowMap;

    uint8_t colorPalette[4][4];
};

void GPU_init(GPU* gpu);
void GPU_destroy(GPU* gpu);
void GPU_update(CPU* cpu, GPU* gpu, Memory* mem);
void GPU_renderToFrameBuffer(GPU* gpu, Memory* mem);

#endif