#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "cartridge.h"
#include "memory.h"

void MEM_init(Memory* mem) {
    // Zero out memory
    for (int i = 0; i < 0x10000; ++i) {
        mem->logicalMemory[i] = 0;
    }

    // Set special registers
    mem->ioRegisters[REG_NR10 - OFFSET_IOREGISTERS] = 0x80;
    mem->ioRegisters[REG_NR11 - OFFSET_IOREGISTERS] = 0xBF;
    mem->ioRegisters[REG_NR12 - OFFSET_IOREGISTERS] = 0xF3;
    mem->ioRegisters[REG_NR14 - OFFSET_IOREGISTERS] = 0xBF;
    mem->ioRegisters[REG_NR21 - OFFSET_IOREGISTERS] = 0x3F;
    mem->ioRegisters[REG_NR24 - OFFSET_IOREGISTERS] = 0xBF;
    mem->ioRegisters[REG_NR30 - OFFSET_IOREGISTERS] = 0x7F;
    mem->ioRegisters[REG_NR31 - OFFSET_IOREGISTERS] = 0xFF;
    mem->ioRegisters[REG_NR32 - OFFSET_IOREGISTERS] = 0x9F;
    mem->ioRegisters[REG_NR34 - OFFSET_IOREGISTERS] = 0xBF;
    mem->ioRegisters[REG_NR41 - OFFSET_IOREGISTERS] = 0xFF;
    mem->ioRegisters[REG_NR44 - OFFSET_IOREGISTERS] = 0xBF;
    mem->ioRegisters[REG_NR50 - OFFSET_IOREGISTERS] = 0x77;
    mem->ioRegisters[REG_NR51 - OFFSET_IOREGISTERS] = 0xF3;
    mem->ioRegisters[REG_NR52 - OFFSET_IOREGISTERS] = 0xF1;
    mem->ioRegisters[REG_LCDC - OFFSET_IOREGISTERS] = 0x91;
    mem->ioRegisters[REG_BGP  - OFFSET_IOREGISTERS] = 0xFC;
    mem->ioRegisters[REG_OBP0 - OFFSET_IOREGISTERS] = 0xFF;
    mem->ioRegisters[REG_OBP1 - OFFSET_IOREGISTERS] = 0xFF;

    // Initialize other variables
    mem->battery = 0;
    mem->dmaAddressUpper = 0;
    mem->dmaInProgress = 0;
    mem->dmaPosition = 0;
}

uint8_t MEM_getByte(Memory* mem, uint16_t address) {
    if (address < OFFSET_ROMBANKN) {
        // ROM bank 0
        return mem->romBank0[address - OFFSET_ROMBANK0];

    } else if (address < OFFSET_VIDEORAM) {
        // ROM bank N
        return mem->romBankN[address - OFFSET_ROMBANKN];

    } else if (address >= OFFSET_EXTRAM && address < OFFSET_WORKRAMBANK0) {
        // External RAM
        return mem->extRamBanksNo != 0 && mem->extRamEnabled
            ? mem->extRam[address - OFFSET_EXTRAM]
            : 0xFF;

    } else {
        return mem->logicalMemory[address];
    }
}

void MEM_setByte(Memory* mem, uint16_t address, uint8_t value) {
    if ((address > 0xDFFF && address < 0xFE00) || (address > 0xFE9F && address < 0xFF00)) {
        //fprintf(stdout, "[MEM] warning: attempt to write to restricted address %04x\n", address);
    } else if (address < OFFSET_VIDEORAM) {
        // Handle MBC operations
        CART_mbcDispatch(mem, address, value);

    } else if (address >= OFFSET_EXTRAM && address < OFFSET_WORKRAMBANK0 && mem->extRamBanksNo != 0 && mem->extRamEnabled) {
        //printf("extram write %d\n", mem->extRamBanksNo);
        //getchar();
        // Set external RAM
        mem->extRam[address - OFFSET_EXTRAM] = value;

    } else if (address == REG_DIV) {
        // Reset the DIV register
        mem->logicalMemory[REG_DIV] = 0;

    } else if (address == REG_DMA) {
        // Initiate a DMA transfer
        MEM_dmaBegin(mem, value);
        mem->logicalMemory[address] = value;

    } else if (address == REG_TAC) {
        // Preserve last 3 bits only (set rest to 1)
        mem->logicalMemory[address] = 0xF8 | (value & 0x7);

    } else {
        mem->logicalMemory[address] = value;
    }
}


void MEM_forceSetByte(Memory* mem, uint16_t address, uint8_t value) {
    mem->logicalMemory[address] = value;
}

void MEM_pushToStack(Memory* mem, uint16_t* SP, uint16_t value) {
    MEM_setByte(mem, *SP - 1, (value & 0xFF00) >> 8);
    MEM_setByte(mem, *SP - 2, value & 0x00FF);
    *SP -= 2;
}

uint16_t MEM_popFromStack(Memory* mem, uint16_t* SP) {
    *SP += 2;
    uint8_t byteUpper = MEM_getByte(mem, *SP - 1);
    uint8_t byteLower = MEM_getByte(mem, *SP - 2);
    return (byteUpper << 8) | byteLower;
}

void MEM_loadROM(Memory* mem, const char* path) {
    // Open file and get size
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        perror("Error while opening ROM");
        exit(1);
    }
    fseek(file, 0L, SEEK_END);
    size_t filesize = ftell(file);
    rewind(file);

    // Load ROM into banks array
    mem->romBanks = malloc(filesize); // freed in main.c:quit
    assert(fread(mem->romBanks, 1, filesize, file) == filesize);
    fclose(file);

    // Load cartridge data
    Cartridge* cart = mem->cartridge;
    mem->cartridge = malloc(sizeof(*cart)); // freed in main.c:quit
    CART_init(mem->cartridge, mem);

    // Set fixed bank to bank 0
    mem->romBank0 = mem->romBanks;

    // Set variable bank to 1
    mem->romBankN = mem->romBanks + 0x4000;

    // Set number of banks
    mem->romBanksNo = ((int[]){2, 4, 8, 16, 32, 64, 128, 256, 512})[mem->cartridge->romSize];
    printf("%d\n", mem->romBanksNo);

    // Initialize external RAM
    mem->extRamEnabled = false;
    mem->extRamBanksNo = ((int[]){0, 0, 1, 4, 16, 8})[mem->cartridge->ramSize];

    if (mem->extRamBanksNo != 0) {
        mem->extRamBanks = calloc(0x2000 * mem->extRamBanksNo, 1);
        mem->extRam = mem->extRamBanks + 0;
    } else {
        mem->extRam = NULL;
    }

    // Handle saving if MBC includes battery
    uint8_t mbcCode = mem->romBank0[0x0147];
    if (mbcCode == 0x03 || mbcCode == 0x06 || mbcCode == 0x09 
        || mbcCode == 0x0D || mbcCode == 0x0F || mbcCode == 0x10 
        || mbcCode == 0x13 || mbcCode == 0x1B || mbcCode == 0x1E 
        || mbcCode == 0x22
    ) {
        mem->battery = 1;
        mem->romPath = malloc(strlen(path) + 1); // freed in main.c:quit
        strcpy(mem->romPath, path);
        char* saveFileName = malloc(strlen(path) + 5); // freed at the end of this block
        snprintf(saveFileName, strlen(path) + 5, "%s.sav", path);
        FILE* saveFile = fopen(saveFileName, "rb");
        if (saveFile != NULL) {
            filesize = mem->extRamBanksNo * 0x2000;
            assert(fread(mem->extRamBanks, 1, filesize, saveFile) == filesize);
            fclose(saveFile);
        }
        free(saveFileName);

    } else {
        mem->battery = 0;
    }
}


static unsigned int intlog2(unsigned int val) {
    if (val == 0) return UINT_MAX;
    if (val == 1) return 0;
    unsigned int ret = 0;
    while (val > 1) {
        val >>= 1;
        ret++;
    }
    return ret;
}

void MEM_setRomBank(Memory* mem, uint8_t bankNo) {
    // Mask the bank number to the required no. of bits
    if (bankNo >= mem->romBanksNo) {
        unsigned int requiredBits = intlog2(mem->romBanksNo);
        uint8_t _mask = (0xFF << requiredBits) & 0xFF;
        uint8_t mask = ~_mask;
        bankNo &= mask;
    }

    mem->romBankN = mem->romBanks + (0x4000 * bankNo);
}

void MEM_setRamBank(Memory* mem, uint8_t bankNo) {
    if (bankNo < mem->extRamBanksNo) {
        mem->extRam = mem->extRamBanks + (0x2000 * bankNo);
    }
}

// Initiate a DMA transfer
void MEM_dmaBegin(Memory* mem, uint8_t addressUpper) {
    mem->dmaAddressUpper = addressUpper;
    mem->dmaPosition = 0x00;
    mem->dmaInProgress = 1;
}

void MEM_dmaUpdate(Memory* mem) {
    if (!mem->dmaInProgress) return;
    uint16_t sourceAddress = (mem->dmaAddressUpper << 8) | mem->dmaPosition;
    uint16_t destAddress = 0xFE00 | mem->dmaPosition;
    MEM_setByte(mem, destAddress, MEM_getByte(mem, sourceAddress));

    if (mem->dmaPosition == 0x9F) {
        mem->dmaPosition = 0x00;
        mem->dmaInProgress = 0;
    } else {
        ++(mem->dmaPosition);
    }
}