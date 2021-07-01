#ifndef CARTRIDGE_H
#define CARTRIDGE_H

typedef struct Cartridge Cartridge;

#include <stdint.h>
#include "memory.h"

struct Cartridge {
    // Header data
    char title[17];
    uint8_t type;
    uint8_t romSize;
    uint8_t ramSize;

    // MBC registers (named according to GBCTR)
    uint8_t RAMG;
    uint8_t BANK1;
    uint8_t BANK2;
    uint8_t MODE;
    uint8_t ROMB;
    uint8_t ROMB0;
    uint8_t ROMB1;
    uint8_t RAMB;
};

void CART_init(Cartridge* cart, Memory* mem);
void CART_destroy(Cartridge* cart);
void CART_mbcDispatch(Memory* mem, uint16_t address, uint8_t value);

#endif