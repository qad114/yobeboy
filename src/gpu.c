#include <stdio.h>
#include <string.h>

#include "common/bitwise.h"
#include "constants.h"
#include "cpu.h"
#include "gpu.h"
#include "memory.h"

static int getColorNumber(Memory* mem, int index, uint16_t paletteAddress) {
    return (MEM_getByte(mem, paletteAddress) & (0b11 << (index * 2))) >> (index * 2);
}

static uint8_t getColorByte(int color) {
    switch (color) {
        case 0: return 0xFF;
        case 1: return 0xC2;
        case 2: return 0x40;
        case 3: return 0x00;
    }
}

static void renderBackground(GPU* gpu, Memory* mem) {
    uint8_t internalFramebuffer[256 * 256];
    uint8_t SCX = MEM_getByte(mem, REG_SCX);
    uint8_t SCY = MEM_getByte(mem, REG_SCY);
    uint8_t LCDC = MEM_getByte(mem, REG_LCDC);

    // Draw the background
    for (int mapY = 0; mapY < 32; ++mapY) {
        for (int mapX = 0; mapX < 32; ++mapX) {
            uint16_t address = (getBit(LCDC, 3) ? 0x9C00 : 0x9800) + (mapY * 0x20) + mapX;
            uint16_t tileAddress = ((LCDC & 0b00010000) >> 4)
                ? 0x8000 + (MEM_getByte(mem, address) * 0x10)
                : 0x9000 + (((int8_t) (MEM_getByte(mem, address))) * 0x10);

            for (int i = 0; i < 16; i += 2) {
                uint8_t byte1 = MEM_getByte(mem, tileAddress + i);
                uint8_t byte2 = MEM_getByte(mem, tileAddress + i + 1);

                uint8_t pixel1 = ((byte2 & 0b10000000) >> 6) | ((byte1 & 0b10000000) >> 7);
                uint8_t pixel2 = ((byte2 & 0b01000000) >> 5) | ((byte1 & 0b01000000) >> 6);
                uint8_t pixel3 = ((byte2 & 0b00100000) >> 4) | ((byte1 & 0b00100000) >> 5);
                uint8_t pixel4 = ((byte2 & 0b00010000) >> 3) | ((byte1 & 0b00010000) >> 4);
                uint8_t pixel5 = ((byte2 & 0b00001000) >> 2) | ((byte1 & 0b00001000) >> 3);
                uint8_t pixel6 = ((byte2 & 0b00000100) >> 1) | ((byte1 & 0b00000100) >> 2);
                uint8_t pixel7 =  (byte2 & 0b00000010)       | ((byte1 & 0b00000010) >> 1);
                uint8_t pixel8 = ((byte2 & 0b00000001) << 1) |  (byte1 & 0b00000001);

                int offset = (mapY * 8 * 256) + (((int)(i / 2)) * 256) + (mapX * 8);
                internalFramebuffer[offset] = pixel1;
                internalFramebuffer[offset + 1] = pixel2;
                internalFramebuffer[offset + 2] = pixel3;
                internalFramebuffer[offset + 3] = pixel4;
                internalFramebuffer[offset + 4] = pixel5;
                internalFramebuffer[offset + 5] = pixel6;
                internalFramebuffer[offset + 6] = pixel7;
                internalFramebuffer[offset + 7] = pixel8;
            }
        }
    }

    int i = 0;
    for (int y = SCY; y < (SCY + 144); ++y) {
        for (int x = SCX; x < (SCX + 160); ++x) {
            uint8_t color = getColorByte(getColorNumber(mem, internalFramebuffer[(((uint8_t) y) * 256) + ((uint8_t) x)], REG_BGP));
            gpu->framebuffer[i] = color;
            gpu->framebuffer[i + 1] = color;
            gpu->framebuffer[i + 2] = color;
            gpu->framebuffer[i + 3] = 255;
            i += 4;
        }
    }
}

static void renderWindow(GPU* gpu, Memory* mem) {
    /*uint8_t internalFramebuffer[256 * 256];
    uint8_t WX = MEM_getByte(mem, REG_WX);
    uint8_t WY = MEM_getByte(mem, REG_WY);
    uint8_t LCDC = MEM_getByte(mem, REG_LCDC);

    // Draw the background
    for (int mapY = 0; mapY < 32; ++mapY) {
        for (int mapX = 0; mapX < 32; ++mapX) {
            uint16_t address = (getBit(LCDC, 6) ? 0x9C00 : 0x9800) + (mapY * 0x20) + mapX;
            uint16_t tileAddress = ((LCDC & 0b00010000) >> 4)
                ? 0x8000 + (MEM_getByte(mem, address) * 0x10)
                : 0x9000 + (((int8_t) (MEM_getByte(mem, address))) * 0x10);

            for (int i = 0; i < 16; i += 2) {
                uint8_t byte1 = MEM_getByte(mem, tileAddress + i);
                uint8_t byte2 = MEM_getByte(mem, tileAddress + i + 1);

                uint8_t pixel1 = ((byte2 & 0b10000000) >> 6) | ((byte1 & 0b10000000) >> 7);
                uint8_t pixel2 = ((byte2 & 0b01000000) >> 5) | ((byte1 & 0b01000000) >> 6);
                uint8_t pixel3 = ((byte2 & 0b00100000) >> 4) | ((byte1 & 0b00100000) >> 5);
                uint8_t pixel4 = ((byte2 & 0b00010000) >> 3) | ((byte1 & 0b00010000) >> 4);
                uint8_t pixel5 = ((byte2 & 0b00001000) >> 2) | ((byte1 & 0b00001000) >> 3);
                uint8_t pixel6 = ((byte2 & 0b00000100) >> 1) | ((byte1 & 0b00000100) >> 2);
                uint8_t pixel7 =  (byte2 & 0b00000010)       | ((byte1 & 0b00000010) >> 1);
                uint8_t pixel8 = ((byte2 & 0b00000001) << 1) |  (byte1 & 0b00000001);

                int offset = (mapY * 8 * 256) + (((int)(i / 2)) * 256) + (mapX * 8);
                internalFramebuffer[offset] = pixel1;
                internalFramebuffer[offset + 1] = pixel2;
                internalFramebuffer[offset + 2] = pixel3;
                internalFramebuffer[offset + 3] = pixel4;
                internalFramebuffer[offset + 4] = pixel5;
                internalFramebuffer[offset + 5] = pixel6;
                internalFramebuffer[offset + 6] = pixel7;
                internalFramebuffer[offset + 7] = pixel8;
            }
        }
    }

    int i = 0;
    for (int y = WY; y < (WY + 144); ++y) {
        for (int x = WX - 7; x < (WX + 153); ++x) {
            uint8_t color = getColorByte(getColorNumber(mem, internalFramebuffer[(((uint8_t) y) * 256) + ((uint8_t) x)], REG_BGP));
            gpu->framebuffer[i] = color;
            gpu->framebuffer[i + 1] = color;
            gpu->framebuffer[i + 2] = color;
            gpu->framebuffer[i + 3] = 255;
            i += 4;
        }
    }*/
}

/*static void renderWindow(GPU* gpu, Memory* mem) {
    uint8_t internalFramebuffer[256 * 256];
    uint8_t WX = MEM_getByte(mem, REG_WX);
    uint8_t WY = MEM_getByte(mem, REG_WY);
    uint8_t LCDC = MEM_getByte(mem, REG_LCDC);

    for (int mapY = 0; mapY < 32; ++mapY) {
        for (int mapX = 0; mapX < 32; ++mapX) {
            uint16_t address = (getBit(LCDC, 6) ? 0x9C00 : 0x9800) + (mapY * 0x20) + mapX;
            uint16_t tileAddress = ((LCDC & 0b00010000) >> 4)
                ? 0x8000 + (MEM_getByte(mem, address) * 0x10)
                : 0x9000 + (((int8_t) (MEM_getByte(mem, address))) * 0x10);

            for (int i = 0; i < 16; i += 2) {
                uint8_t byte1 = MEM_getByte(mem, tileAddress + i);
                uint8_t byte2 = MEM_getByte(mem, tileAddress + i + 1);

                uint8_t pixel1 = ((byte2 & 0b10000000) >> 6) | ((byte1 & 0b10000000) >> 7);
                uint8_t pixel2 = ((byte2 & 0b01000000) >> 5) | ((byte1 & 0b01000000) >> 6);
                uint8_t pixel3 = ((byte2 & 0b00100000) >> 4) | ((byte1 & 0b00100000) >> 5);
                uint8_t pixel4 = ((byte2 & 0b00010000) >> 3) | ((byte1 & 0b00010000) >> 4);
                uint8_t pixel5 = ((byte2 & 0b00001000) >> 2) | ((byte1 & 0b00001000) >> 3);
                uint8_t pixel6 = ((byte2 & 0b00000100) >> 1) | ((byte1 & 0b00000100) >> 2);
                uint8_t pixel7 =  (byte2 & 0b00000010)       | ((byte1 & 0b00000010) >> 1);
                uint8_t pixel8 = ((byte2 & 0b00000001) << 1) |  (byte1 & 0b00000001);

                int offset = (mapY * 8 * 256) + (((int)(i / 2)) * 256) + (mapX * 8);
                internalFramebuffer[offset] = pixel1;
                internalFramebuffer[offset + 1] = pixel2;
                internalFramebuffer[offset + 2] = pixel3;
                internalFramebuffer[offset + 3] = pixel4;
                internalFramebuffer[offset + 4] = pixel5;
                internalFramebuffer[offset + 5] = pixel6;
                internalFramebuffer[offset + 6] = pixel7;
                internalFramebuffer[offset + 7] = pixel8;
            }
        }
    }

    int offset = (WY * 4 * 160) + ((WX - 7) * 4);
    gpu->framebuffer[offset] = 0xFF;
    gpu->framebuffer[offset + 1] = 0x00;
    gpu->framebuffer[offset + 2] = 0x00;
    gpu->framebuffer[offset + 3] = 0xFF;

    int i = 0;
    for (int y = WY; y < (WY + 144); ++y) {
        for (int x = WX - 7; x < (WX + 153); ++x) {
            uint8_t color = getColorByte(getColorNumber(mem, internalFramebuffer[(((uint8_t) y) * 256) + ((uint8_t) x)], REG_BGP));
            gpu->framebuffer[i] = color;
            gpu->framebuffer[i + 1] = color;
            gpu->framebuffer[i + 2] = color;
            gpu->framebuffer[i + 3] = 255;
            i += 4;
        }
    }
}*/



static void renderObjects(GPU* gpu, Memory* mem) {
    for (int i = 0xFE00; i < 0xFE9F; i += 4) {
        int yPos = MEM_getByte(mem, i) - 16;
        int xPos = MEM_getByte(mem, i + 1) - 8;
        int tileIndex = MEM_getByte(mem, i + 2);
        uint16_t tileAddress = 0x8000 + (tileIndex * 0x10);
        uint8_t flags = MEM_getByte(mem, i + 3);
        int underBg = getBit(flags, 7);
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
                int color = getColorNumber(mem, pixels[i], (flags & 0b00010000) >> 4 ? REG_OBP1 : REG_OBP0);
                int pos = offset + (i * 4);
                if (pos < 0) continue;
                memset(gpu->framebuffer + pos, getColorByte(color), 3);
                gpu->framebuffer[offset + (i * 4) + 3] = 0xFF;
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
            MEM_setByte(mem, REG_STAT, (STAT & 0xFC) | 1);
            GPU_vRamToFrameBuffer(gpu, mem);
            MEM_setByte(mem, REG_IF, MEM_getByte(mem, REG_IF) | 0x1);
            MEM_setByte(mem, REG_LY, LY + 1);

        } else if (LY == 153) {
            // VBlank period ended
            uint8_t STAT = MEM_getByte(mem, REG_STAT);
            MEM_setByte(mem, REG_STAT, (STAT & 0xFC) | 2);
            MEM_setByte(mem, REG_LY, 0);

        } else {
            // Go to next line
            MEM_setByte(mem, REG_LY, LY + 1);
        }
    } else {
        ++(gpu->machineCycleCounter);
    }

    // Update STAT register mid-frame
    uint8_t STAT = MEM_getByte(mem, REG_STAT);
    if (gpu->machineCycleCounter == 20) {
        MEM_setByte(mem, REG_STAT, (STAT & 0xFC) | 3);
    } else if (gpu->machineCycleCounter == 78) {
        MEM_setByte(mem, REG_STAT, (STAT & 0xFC) | 0);
    }

    // Check if LYC=LY. If true, request STAT interrupt
    STAT = MEM_getByte(mem, REG_STAT);
    if (MEM_getByte(mem, REG_LYC) == MEM_getByte(mem, REG_LY)) {
        MEM_setByte(mem, REG_STAT, setBit(STAT, 2, 1));
        MEM_setByte(mem, REG_IF, MEM_getByte(mem, REG_IF) | 0b10);
    } else {
        MEM_setByte(mem, REG_STAT, setBit(STAT, 2, 0));
    }
}

// Generate LCD framebuffer from VRAM
void GPU_vRamToFrameBuffer(GPU* gpu, Memory* mem) {
    uint8_t LCDC = MEM_getByte(mem, REG_LCDC);
    if (!((LCDC & 0b10000000) >> 7)) {
        // LCD disabled, return a blank screen
        for (int i = 0; i < (160 * 144 * 4); ++i) {
            gpu->framebuffer[i] = 0xFF;
            gpu->fbUpdated = 1;
        }
        return;
    }

    if (LCDC & 0b1) {renderBackground(gpu, mem); renderWindow(gpu, mem);};
    if ((LCDC & 0b10) >> 1) renderObjects(gpu, mem);
    gpu->fbUpdated = 1;
}