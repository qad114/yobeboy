#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/bitwise.h"
#include "constants.h"
#include "cpu.h"
#include "gpu.h"
#include "memory.h"

void GPU_init(GPU* gpu) {
    gpu->framebuffer = malloc(sizeof(uint8_t) * GB_SCREEN_WIDTH * GB_SCREEN_HEIGHT * 4 + 512); // freed in main.c:quit
    gpu->fbUpdated = false;
    gpu->machineCycleCounter = 0;

    gpu->backgroundMap = malloc(32 * 32 * 16);
}

static int getColorNumber(Memory* mem, int index, uint16_t paletteAddress) {
    return (MEM_getByte(mem, paletteAddress) & (0x3 << (index * 2))) >> (index * 2);
}

static uint8_t getColorByte(int color) {
    return (uint8_t []){0xFF, 0xC2, 0x40, 0x00}[color];
}

static void updateBackgroundMap(GPU* gpu, Memory* mem) {
    uint8_t LCDC = MEM_getByte(mem, REG_LCDC);

    for (int mapY = 0; mapY < 32; ++mapY) {
        for (int mapX = 0; mapX < 32; ++mapX) {
            uint16_t address = (getBit(LCDC, 3) ? 0x9C00 : 0x9800) + (mapY * 0x20) + mapX;
            uint16_t tileAddress = getBit(LCDC, 4)
                ? 0x8000 + (MEM_getByte(mem, address) * 0x10)
                : 0x9000 + (((int8_t) (MEM_getByte(mem, address))) * 0x10);

            uint8_t* srcAddress = mem->videoRam + tileAddress - OFFSET_VIDEORAM;
            uint8_t* targetAddress = gpu->backgroundMap + (mapY * 16 * 32) + (mapX * 16);
            memcpy(targetAddress, srcAddress, 16);
        }
    }
}

static void renderBgScanline(GPU* gpu, Memory* mem, int line) {
    if (line >= GB_SCREEN_HEIGHT) return; // VBlank period
    uint8_t SCX = MEM_getByte(mem, REG_SCX);
    uint8_t SCY = MEM_getByte(mem, REG_SCY);

    int tileXOffset = SCX / 8, tileYOffset = (SCY + line) / 8;
    int pixelXOffset = SCX % 8, pixelYOffset = (SCY + line) % 8;
    if (tileYOffset >= 32) tileYOffset -= 32;

    uint8_t* linePtr = gpu->framebuffer + (line * GB_SCREEN_WIDTH * 4);
    uint8_t* lineEndPtr = linePtr + (GB_SCREEN_WIDTH * 4);

    uint8_t* mapStartPtr = gpu->backgroundMap + (tileYOffset * 16 * 32);
    uint8_t* mapPtr = mapStartPtr + (tileXOffset * 16);

    // Draw the first few pixels separately
    uint8_t* firstChunk = mapPtr + (pixelYOffset * 2);
    uint8_t byte1 = firstChunk[0];
    uint8_t byte2 = firstChunk[1];
    for (int i = pixelXOffset; i < 8; ++i) {
        int color = getBit(byte2, 7 - i) << 1 | getBit(byte1, 7 - i);
        memset(linePtr + ((i - pixelXOffset) * 4), getColorByte(getColorNumber(mem, color, REG_BGP)), 3);
        linePtr[((i - pixelXOffset) * 4) + 3] = 0xFF;
    }
    linePtr += ((8 - pixelXOffset) * 4);

    // Draw the remaining pixels
    for (int tileNo = tileXOffset + 1; linePtr < lineEndPtr; ++tileNo) {
        if (tileNo == 32) tileNo = 0;
        uint8_t* chunk = mapStartPtr + (tileNo * 16) + (pixelYOffset * 2);
        for (int i = 0; i < 8; ++i) {
            int color = getBit(chunk[1], 7 - i) << 1 | getBit(chunk[0], 7 - i);
            memset(linePtr + ((i) * 4), getColorByte(getColorNumber(mem, color, REG_BGP)), 3);
            linePtr[((i) * 4) + 3] = 0xFF;
        }
        linePtr += (8 * 4);
    }
}

static void renderWindow(GPU* gpu, Memory* mem) {
    uint8_t* internalFramebuffer = malloc(256 * 256); // freed at the end of this function
    uint8_t WX = MEM_getByte(mem, REG_WX);
    uint8_t WY = MEM_getByte(mem, REG_WY);
    uint8_t LCDC = MEM_getByte(mem, REG_LCDC);

    for (int mapY = 0; mapY < 32; ++mapY) {
        for (int mapX = 0; mapX < 32; ++mapX) {
            uint16_t address = (getBit(LCDC, 6) ? 0x9C00 : 0x9800) + (mapY * 0x20) + mapX;
            uint16_t tileAddress = getBit(LCDC, 4)
                ? 0x8000 + (MEM_getByte(mem, address) * 0x10)
                : 0x9000 + (((int8_t) (MEM_getByte(mem, address))) * 0x10);

            for (int i = 0; i < 16; i += 2) {
                uint8_t byte1 = MEM_getByte(mem, tileAddress + i);
                uint8_t byte2 = MEM_getByte(mem, tileAddress + i + 1);
                
                int offset = (mapY * 8 * 256) + (((int)(i / 2)) * 256) + (mapX * 8);
                for (int i = 0; i < 8; ++i) {
                    internalFramebuffer[offset + i] = getBit(byte2, 7 - i) << 1 | getBit(byte1, 7 - i);
                }
            }
        }
    }

    int xLowerBound = (WX - 7) < 0 ? 0 : (WX - 7);
    for (int y = WY; y < 144; ++y) {
        for (int x = xLowerBound; x < 160; ++x) {
            int pos = (y * 160 * 4) + (x * 4);
            uint8_t color = getColorByte(getColorNumber(mem, internalFramebuffer[((y - WY) * 256) + (x - WX + 7)], REG_BGP));
            gpu->framebuffer[pos + 0] = color;
            gpu->framebuffer[pos + 1] = color;
            gpu->framebuffer[pos + 2] = color;
            gpu->framebuffer[pos + 3] = 0xFF;
        }
    }

    gpu->fbUpdated = true;
    free(internalFramebuffer);
}


static void renderObjects(GPU* gpu, Memory* mem) {
    for (int i = 0xFE00; i < 0xFE9F; i += 4) {
        int yPos = MEM_getByte(mem, i) - 16;
        int xPos = MEM_getByte(mem, i + 1) - 8;
        if (xPos >= 160) continue;

        int tileIndex = MEM_getByte(mem, i + 2);
        uint16_t tileAddress = 0x8000 + (tileIndex * 0x10);
        uint8_t flags = MEM_getByte(mem, i + 3);
        //int underBg = getBit(flags, 7);
        int yFlip = getBit(flags, 6);
        int xFlip = getBit(flags, 5);

        //if (underBg) continue;

        uint8_t longObjectMode = getBit(MEM_getByte(mem, REG_LCDC), 2); // Is 8x16 mode active?
        int rowIndex = yFlip ? (longObjectMode ? 31 : 15) : 0;
        for (int i = 0; i < (longObjectMode ? 32 : 16); i += 2) {
            uint8_t byte1 = MEM_getByte(mem, tileAddress + i);
            uint8_t byte2 = MEM_getByte(mem, tileAddress + i + 1);

            uint8_t pixels[8];
            int pixelIndex = (xFlip ? 7 : 0);
            for (int i = 0; i < 8; ++i) {
                pixels[pixelIndex] = getBit(byte2, 7 - i) << 1 | getBit(byte1, 7 - i);
                pixelIndex += (xFlip ? -1 : 1);
            }

            int offset = (yPos * 4 * 160) + ((rowIndex / 2) * 4 * 160) + (xPos * 4);
            for (int i = 0; i < 8; ++i) {
                if (pixels[i] == 0) continue;
                int pos = offset + (i * 4);
                if (pos < 0 || pos >= (160 * 144 * 4) || xPos + i < 0 || xPos + i >= 160) continue; // TODO: Removing the first check should work, but doesn't - possible bug
                int color = getColorNumber(mem, pixels[i], (getBit(flags, 4) ? REG_OBP1 : REG_OBP0));
                memset(gpu->framebuffer + pos, getColorByte(color), 3);
                gpu->framebuffer[pos + 3] = 0xFF;
            }

            rowIndex += (yFlip ? -2 : 2);
        }
    }
}

// Update the GPU and display registers' states (runs every machine cycle)
void GPU_update(CPU* cpu, GPU* gpu, Memory* mem) {
    // Update LY (every 114 cycles)
    if (gpu->machineCycleCounter == 114) {
        gpu->machineCycleCounter = 0;
        uint8_t LY = MEM_getByte(mem, REG_LY);

        if (LY == 144) {
            // VBlank period started
            uint8_t STAT = MEM_getByte(mem, REG_STAT);
            MEM_setByte(mem, REG_STAT, (STAT & 0xFC) | 1); // Mode 1 (VBlank)
            GPU_renderToFrameBuffer(gpu, mem);
            MEM_setByte(mem, REG_IF, MEM_getByte(mem, REG_IF) | 0x1);
            MEM_setByte(mem, REG_LY, LY + 1);

        } else if (LY == 153) {
            // VBlank period ended
            uint8_t STAT = MEM_getByte(mem, REG_STAT);
            MEM_setByte(mem, REG_STAT, (STAT & 0xFC) | 2); // Mode 2
            MEM_setByte(mem, REG_LY, 0);

        } else {
            // Go to next line
            MEM_setByte(mem, REG_LY, LY + 1);
        }
    } else {
        ++(gpu->machineCycleCounter);
    }

    // Update STAT register mid-scanline
    uint8_t STAT = MEM_getByte(mem, REG_STAT);
    if (gpu->machineCycleCounter == 20) {
        MEM_setByte(mem, REG_STAT, (STAT & 0xFC) | 3); // Mode 3
    } else if (gpu->machineCycleCounter == 62) {
        MEM_setByte(mem, REG_STAT, (STAT & 0xFC) | 0); // Mode 0 (HBlank)
        updateBackgroundMap(gpu, mem);
        renderBgScanline(gpu, mem, MEM_getByte(mem, REG_LY));
    }

    // Check if LYC=LY. If true, request STAT interrupt
    STAT = MEM_getByte(mem, REG_STAT);
    if (MEM_getByte(mem, REG_LYC) == MEM_getByte(mem, REG_LY)) {
        MEM_setByte(mem, REG_STAT, setBit(STAT, 2, 1));
        MEM_setByte(mem, REG_IF, setBit(MEM_getByte(mem, REG_IF), 1, 1));
    } else {
        MEM_setByte(mem, REG_STAT, setBit(STAT, 2, 0));
    }
}

// Generate LCD framebuffer from VRAM
void GPU_renderToFrameBuffer(GPU* gpu, Memory* mem) {
    uint8_t LCDC = MEM_getByte(mem, REG_LCDC);
    if (!getBit(LCDC, 7)) {
        // LCD disabled, return a blank screen
        for (int i = 0; i < (160 * 144 * 4); ++i) {
            gpu->framebuffer[i] = 0xFF;
            gpu->fbUpdated = true;
        }
        return;
    }

    if (getBit(LCDC, 0)) {
        //renderBackground(gpu, mem);
        if (getBit(LCDC, 5)) renderWindow(gpu, mem);
    };
    if (getBit(LCDC, 1)) renderObjects(gpu, mem);
    gpu->fbUpdated = true;
}