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
    return (uint8_t []){0xFF, 0xC2, 0x40, 0x00}[color];
}

static void renderBackground(GPU* gpu, Memory* mem) {
    uint8_t* internalFramebuffer = malloc(256 * 256);
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
                internalFramebuffer[offset + 0] = pixel1;
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

    free(internalFramebuffer);
}

// TODO: This function aims to render the background faster, but it is currently
// incomplete (and I may never complete it as I might move onto scanline rendering)
static void UNUSED_renderBackground(GPU* gpu, Memory* mem) {
    uint8_t SCX = MEM_getByte(mem, REG_SCX);
    uint8_t SCY = MEM_getByte(mem, REG_SCY);
    uint8_t LCDC = MEM_getByte(mem, REG_LCDC);

    for (int mapY = SCY / 8; mapY < (SCY + 144) / 8; ++mapY) {
        for (int mapX = SCX / 8; mapX < (SCX + 160) / 8; ++mapX) {
            uint16_t address = (getBit(LCDC, 3) ? 0x9C00 : 0x9800) + (mapY * 0x20) + mapX;
            uint16_t tileAddress = ((LCDC & 0b00010000) >> 4)
                ? 0x8000 + (MEM_getByte(mem, address) * 0x10)
                : 0x9000 + (((int8_t) (MEM_getByte(mem, address))) * 0x10);

            for (int i = 0; i < 16; i += 2) {
                uint8_t byte1 = MEM_getByte(mem, tileAddress + i);
                uint8_t byte2 = MEM_getByte(mem, tileAddress + i + 1);

                uint8_t pixels[8];
                for (int i = 0; i < 8; ++i) {
                    pixels[i] = getBit(byte2, 7 - i) << 1 | getBit(byte1, 7 - i);
                }

                int offset = (((mapY * 8) - SCY) * 160 * 4) + ((i / 2) * 160 * 4) + (((mapX * 8) - SCX) * 4) + ((SCX % 8) * 4);// - (32 - ((SCX % 8) * 4));
                for (int i = 0; i < 32; i += 4) {
                    int loc = offset + i;
                    int xPos = (((mapX * 8) - SCX) * 4);
                    if (loc < 0 || loc > (160 * 144 * 4) || xPos > (160 * 4) || xPos < 0) continue;
                    //if (loc < 0 || loc > (160 * 144 * 4)) continue;
                    memset(gpu->framebuffer + loc, getColorByte(getColorNumber(mem, pixels[i / 4], REG_BGP)), 3);
                    gpu->framebuffer[loc + 3] = 0xFF;
                }
            }
        }
    }
}

static void renderWindow(GPU* gpu, Memory* mem) {
    uint8_t* internalFramebuffer = malloc(256 * 256);
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
                internalFramebuffer[offset + 0] = pixel1;
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

    for (int y = WY; y < 144; ++y) {
        for (int x = (WX - 7); x < 160; ++x) {
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
                int pos = offset + (i * 4);
                if (pos < 0 || pos >= (160 * 144 * 4) || xPos + i < 0 || xPos + i >= 160) continue; // TODO: Removing the first check should work, but doesn't - possible bug
                int color = getColorNumber(mem, pixels[i], (flags & 0b00010000) >> 4 ? REG_OBP1 : REG_OBP0);
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
            GPU_vRamToFrameBuffer(gpu, mem);
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
            gpu->fbUpdated = true;
        }
        return;
    }

    if (getBit(LCDC, 1)) {
        renderBackground(gpu, mem);
        if (getBit(LCDC, 5)) renderWindow(gpu, mem);
    };
    if ((LCDC & 0x2) >> 1) renderObjects(gpu, mem);
    gpu->fbUpdated = true;
}