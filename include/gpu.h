#ifndef GPU_H
#define GPU_H

typedef struct GPU GPU;

#include "cpu.h"
#include "memory.h"

struct GPU {
    int machineCycleCounter;
    uint8_t* framebuffer;
    int fbUpdated;
};

static int getColorNumber(Memory* mem, int index, uint16_t paletteAddress);
static uint8_t getColorByte(int color);
static void renderBackground(GPU* gpu, Memory* mem);
static void renderObjects(GPU* gpu, Memory* mem);

void GPU_update(CPU* cpu, GPU* gpu, Memory* mem);
void GPU_vRamToFrameBuffer(GPU* gpu, Memory* mem);

#endif