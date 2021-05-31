#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "memory.h"

void MEM_init(Memory* mem) {
    // Map memory sections to logical memory array
    for (int i = OFFSET_ROMBANK0; i < OFFSET_ROMBANKN; ++i) mem->logicalMemory[i] = &(mem->romBank0[i - OFFSET_ROMBANK0]);
    for (int i = OFFSET_ROMBANKN; i < OFFSET_VIDEORAM; ++i) mem->logicalMemory[i] = &(mem->romBankN[i - OFFSET_ROMBANKN]);
    for (int i = OFFSET_VIDEORAM; i < OFFSET_EXTRAM; ++i) mem->logicalMemory[i] = &(mem->videoRam[i - OFFSET_VIDEORAM]);
    for (int i = OFFSET_EXTRAM; i < OFFSET_WORKRAMBANK0; ++i) mem->logicalMemory[i] = &(mem->extRam[i - OFFSET_EXTRAM]);
    for (int i = OFFSET_WORKRAMBANK0; i < OFFSET_WORKRAMBANK1; ++i) mem->logicalMemory[i] = &(mem->workRamBank0[i - OFFSET_WORKRAMBANK0]);
    for (int i = OFFSET_WORKRAMBANK1; i < OFFSET_ECHORAM; ++i) mem->logicalMemory[i] = &(mem->workRamBank1[i - OFFSET_WORKRAMBANK1]);
    for (int i = OFFSET_ECHORAM; i < OFFSET_SPRITEATTRIBUTETABLE; ++i) mem->logicalMemory[i] = &(mem->echoRam[i - OFFSET_ECHORAM]);
    for (int i = OFFSET_SPRITEATTRIBUTETABLE; i < OFFSET_UNUSABLE; ++i) mem->logicalMemory[i] = &(mem->spriteAttributeTable[i - OFFSET_SPRITEATTRIBUTETABLE]);
    for (int i = OFFSET_UNUSABLE; i < OFFSET_IOREGISTERS; ++i) mem->logicalMemory[i] = &(mem->unusable[i - OFFSET_UNUSABLE]);
    for (int i = OFFSET_IOREGISTERS; i < OFFSET_HIGHRAM; ++i) mem->logicalMemory[i] = &(mem->ioRegisters[i - OFFSET_IOREGISTERS]);
    for (int i = OFFSET_HIGHRAM; i < REG_IE; ++i) mem->logicalMemory[i] = &(mem->highRam[i - OFFSET_HIGHRAM]);
    mem->logicalMemory[0xFFFF] = &(mem->IE);

    // Zero out memory
    for (int i = OFFSET_VIDEORAM; i < OFFSET_EXTRAM; ++i) {
        *(mem->logicalMemory[i]) = 0;
    }
    for (int i = OFFSET_WORKRAMBANK0; i <= 0xFFFF; ++i) {
        *(mem->logicalMemory[i]) = 0;
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
    mem->ioRegisters[REG_BGP  - OFFSET_IOREGISTERS]  = 0xFC;
    mem->ioRegisters[REG_OBP0 - OFFSET_IOREGISTERS] = 0xFF;
    mem->ioRegisters[REG_OBP1 - OFFSET_IOREGISTERS] = 0xFF;

    // Initialize other variables
    mem->battery = 0;
    mem->dmaAddressUpper = 0;
    mem->dmaInProgress = 0;
    mem->dmaPosition = 0;
}

uint8_t MEM_getByte(Memory* mem, uint16_t address) {
    if (address >= OFFSET_EXTRAM && address < OFFSET_WORKRAMBANK0 && mem->extRamBanksNo == 0) {
        return 0xFF;
    }
    return *(mem->logicalMemory[address]);
}

/*uint8_t* MEM_getReference(Memory* mem, uint16_t address) {
    return &(mem->data[address]);
}*/

void MEM_setByte(Memory* mem, uint16_t address, uint8_t value) {
    if ((address > 0xDFFF && address < 0xFE00) || (address > 0xFE9F && address < 0xFF00)) {
        //fprintf(stdout, "[MEM] warning: attempt to write to restricted address %04x\n", address);
    } else if (address < OFFSET_VIDEORAM) {
        // Handle MBC operations
        MEM_MBC_dispatch(mem, address, value);

    } else if (address == REG_DIV) {
        // Reset the DIV register
        *(mem->logicalMemory[REG_DIV]) = 0;

    } else if (address == REG_DMA) {
        // Initiate a DMA transfer
        MEM_dmaBegin(mem, value);
        *(mem->logicalMemory[address]) = value;

    } else if (address >= OFFSET_EXTRAM && address < OFFSET_WORKRAMBANK0 && mem->extRamBanksNo == 0) {
        // Do nothing

    } else {
        /*if (address == 0xBFFF) {
            printf("bfff\n");
            getchar();
            printf("%d\n", *(mem->logicalMemory[address]));
            getchar();
        }*/
        *(mem->logicalMemory[address]) = value;
    }
}


void MEM_forceSetByte(Memory* mem, uint16_t address, uint8_t value) {
    *(mem->logicalMemory[address]) = value;
}

void MEM_pushToStack(Memory* mem, uint16_t* SP, uint16_t value) {
    //mem->data[*SP - 1] = (value & 0xFF00) >> 8;
    //mem->data[*SP - 2] = value & 0x00FF;
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
        perror("error while opening rom");
        exit(1);
    }
    fseek(file, 0L, SEEK_END);
    int filesize = ftell(file);
    rewind(file);
    mem->romBanksNo = filesize / 0x4000;
    printf("%d\n", mem->romBanksNo);

    // Load ROM into banks array
    mem->romBanks = malloc(filesize);
    assert(fread(mem->romBanks, 1, filesize, file) == filesize);
    fclose(file);

    // Set fixed bank to bank 0
    mem->romBank0 = mem->romBanks;

    // Set variable bank to 1
    MEM_setRomBank(mem, 1);

    // Initialize external RAM
    mem->extRamEnabled = 0;
    mem->extRamAllocated = 0;

    switch (mem->romBank0[0x0149]) {
        case 0: mem->extRamBanksNo = 0; break;
        case 1: mem->extRamBanksNo = 0; break;
        case 2: mem->extRamBanksNo = 1; break;
        case 3: mem->extRamBanksNo = 4; break;
        case 4: mem->extRamBanksNo = 16; break;
        case 5: mem->extRamBanksNo = 8; break;
        default: mem->extRamBanksNo = 0; break;
    }

    if (mem->extRamBanksNo != 0) {
        mem->extRamBanks = calloc(0x2000 * mem->extRamBanksNo, 1);
        mem->extRam = mem->extRamBanks + 0;
    } else {
        mem->extRam = NULL;
    }

    // Handle saving if MBC includes battery
    uint8_t mbcCode = mem->romBank0[0x0147];
    if (mbcCode == 0x03) {
        mem->battery = 1;
        strcpy(mem->romPath, path);
        char saveFileName[128];
        sprintf(saveFileName, "%s.sav", path);
        FILE* saveFile = fopen(saveFileName, "rb");
        if (saveFile != NULL) {
            filesize = mem->extRamBanksNo * 0x2000;
            assert(fread(mem->extRamBanks, 1, filesize, saveFile) == filesize);
            fclose(saveFile);
        }
    } else {
        mem->battery = 0;
    }

    // Remap logical addresses
    for (int i = OFFSET_ROMBANK0; i < OFFSET_ROMBANKN; ++i) mem->logicalMemory[i] = &(mem->romBank0[i - OFFSET_ROMBANK0]);
    for (int i = OFFSET_ROMBANKN; i < OFFSET_VIDEORAM; ++i) mem->logicalMemory[i] = &(mem->romBankN[i - OFFSET_ROMBANKN]);
    for (int i = OFFSET_EXTRAM; i < OFFSET_WORKRAMBANK0; ++i) mem->logicalMemory[i] = &(mem->extRam[i - OFFSET_EXTRAM]);
}

void MEM_setRomBank(Memory* mem, int bank) {
    mem->romBankN = mem->romBanks + (0x4000 * bank);
    for (int i = OFFSET_ROMBANKN; i < OFFSET_VIDEORAM; ++i) mem->logicalMemory[i] = &(mem->romBankN[i - OFFSET_ROMBANKN]);
}

void MEM_setRamBank(Memory* mem, int bank) {
    mem->extRam = mem->extRamBanks + (0x2000 * bank);
    for (int i = OFFSET_EXTRAM; i < OFFSET_WORKRAMBANK0; ++i) mem->logicalMemory[i] = &(mem->extRam[i - OFFSET_EXTRAM]);
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

void MEM_MBC_dispatch(Memory* mem, uint16_t address, uint8_t value) {
    switch (MEM_getByte(mem, 0x0147)) {
        case 0x00: // ROM ONLY
            // Ignore write attempt
            break;

        case 0x01: // MBC1
            MEM_MBC1_handle(mem, address, value);
            break;

        case 0x02: // MBC1+RAM
            MEM_MBC1_RAM_handle(mem, address, value);
            break;

        case 0x03: // MBC1+RAM+BATTERY
            mem->battery = 1;
            MEM_MBC1_RAM_handle(mem, address, value);
            break;

        /*case 0x13:
            MEM_MBC1_RAM_handle(mem, address, value);
            break;*/

        default:
            printf("Unimplemented MBC: %x\n", MEM_getByte(mem, 0x0147));
            break;
    }
}

static uint8_t getBankNo(uint8_t bankNoByte, int totalBanks) {
    uint8_t bankNo = bankNoByte & 0b00011111;
    if (bankNo >= totalBanks) {
        int requiredBits = 0;
        uint8_t bankNoCopy = bankNo;
        while (bankNoCopy != 0) {
            ++requiredBits;
            bankNoCopy = bankNoCopy >> 1;
        }
        bankNo &= ~(1 << requiredBits);
    }
    if (bankNo == 0) bankNo = 1;
    return bankNo;
}

void MEM_MBC1_handle(Memory* mem, uint16_t address, uint8_t value) {
    if (address >= 0x2000 && address <= 0x3FFF) {
        // Change the ROM bank number
        uint8_t bankNo = getBankNo(value, mem->romBanksNo);
        MEM_setRomBank(mem, bankNo);

    } else if (address >= 0x6000 && address <= 0x7FFF) {
        // Select the banking mode (TODO)
    }
}

void MEM_MBC1_RAM_handle(Memory* mem, uint16_t address, uint8_t value) {
    if (address <= 0x1FFF) {
        // Enable external RAM
        mem->extRamEnabled = (value == 0x0A) ? 1 : 0;

    } else if (address >= 0x2000 && address <= 0x3FFF) {
        // Change the ROM bank number
        uint8_t bankNo = getBankNo(value, mem->romBanksNo);
        MEM_setRomBank(mem, bankNo);

    } else if (address >= 0x4000 && address <= 0x5FFF) {
        // Change the RAM bank number
        uint8_t bankNo = value & 0b11;
        MEM_setRamBank(mem, bankNo);

    } else if (address >= 0x6000 && address <= 0x7FFF) {
        // Select the banking mode (TODO)
    }
}

void MEM_MBC3_RAM_handle(Memory* mem, uint16_t address, uint8_t value) {
    if (address <= 0x1FFF) {
        // Enable external RAM
        mem->extRamEnabled = (value == 0x0A) ? 1 : 0;

    } else if (address >= 2000 && address <= 0x3FFF) {
        
    }
}