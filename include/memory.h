#ifndef MEMORY_H
#define MEMORY_H

typedef struct Memory Memory;

#include <stdint.h>

struct Memory {
    uint8_t* logicalMemory[65536];

    uint8_t* romBank0;
    uint8_t* romBankN;
    uint8_t videoRam[0x1FFF + 1];
    uint8_t* extRam;
    uint8_t workRamBank0[0x0FFF + 1];
    uint8_t workRamBank1[0x0FFF + 1];
    uint8_t echoRam[0x1DFF + 1];
    uint8_t spriteAttributeTable[0x009F + 1];
    uint8_t unusable[0x005F + 1];
    uint8_t ioRegisters[0x007F + 1];
    uint8_t highRam[0x007E + 1];
    uint8_t IE;

    uint8_t* romBanks;
    int romBanksNo;
    uint8_t* extRamBanks;
    int extRamBanksNo;
    int extRamEnabled;
    int extRamAllocated;

    int battery;
    char romPath[128];

    int dmaInProgress;
    uint8_t dmaAddressUpper;
    uint8_t dmaPosition;
};

void MEM_init(Memory* mem);
uint8_t MEM_getByte(Memory* mem, uint16_t address);
//uint8_t* MEM_getReference(Memory* mem, uint16_t address);
void MEM_setByte(Memory* mem, uint16_t address, uint8_t value);
void MEM_forceSetByte(Memory* mem, uint16_t address, uint8_t value);
void MEM_pushToStack(Memory* mem, uint16_t* SP, uint16_t value);
uint16_t MEM_popFromStack(Memory* mem, uint16_t* SP);
void MEM_loadROM(Memory* mem, const char* path);
void MEM_setRomBank(Memory* mem, int bank);
void MEM_setRamBank(Memory* mem, int bank);
void MEM_dmaBegin(Memory* mem, uint8_t addressUpper);
void MEM_dmaUpdate(Memory* mem);

void MEM_MBC_dispatch(Memory* mem, uint16_t address, uint8_t value);
void MEM_MBC1_handle(Memory* mem, uint16_t address, uint8_t value);
void MEM_MBC1_RAM_handle(Memory* mem, uint16_t address, uint8_t value);
void MEM_MBC3_RAM_handle(Memory* mem, uint16_t address, uint8_t value);

#endif