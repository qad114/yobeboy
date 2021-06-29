#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/bitwise.h"
#include "cartridge.h"
#include "memory.h"

void CART_init(Cartridge* cart, Memory* mem) {
    // Set header data
    memcpy(cart->title, mem->romBanks + 0x0134, 16);
    cart->title[16] = '\0';
    cart->type = mem->romBanks[0x0147];
    cart->romSize = mem->romBanks[0x0148];
    cart->ramSize = mem->romBanks[0x0149];

    // Set default register values (usually 0)
    cart->RAMG = 0;
    cart->BANK1 = 1;
    cart->BANK2 = 0;
    cart->MODE = 0;
    cart->ROMB = 0;
    cart->ROMB0 = 0;
    cart->ROMB1 = 0;
    cart->RAMB = 0;
}

void CART_destroy(Cartridge* cart) {
    free(cart);
    cart = NULL;
}

static void MBC1(Memory* mem, uint16_t address, uint8_t value);
static void MBC1_RAM(Memory* mem, uint16_t address, uint8_t value);
static void MBC1_RAM_BATTERY(Memory* mem, uint16_t address, uint8_t value);
static void MBC2(Memory* mem, uint16_t address, uint8_t value);
static void MBC2_BATTERY(Memory* mem, uint16_t address, uint8_t value);
static void ROM_RAM(Memory* mem, uint16_t address, uint8_t value);
static void ROM_RAM_BATTERY(Memory* mem, uint16_t address, uint8_t value);
static void MMM01(Memory* mem, uint16_t address, uint8_t value);
static void MMM01_RAM(Memory* mem, uint16_t address, uint8_t value);
static void MMM01_RAM_BATTERY(Memory* mem, uint16_t address, uint8_t value);
static void MBC3_TIMER_BATTERY(Memory* mem, uint16_t address, uint8_t value);
static void MBC3_TIMER_RAM_BATTERY(Memory* mem, uint16_t address, uint8_t value);
static void MBC3(Memory* mem, uint16_t address, uint8_t value);
static void MBC3_RAM(Memory* mem, uint16_t address, uint8_t value);
static void MBC3_RAM_BATTERY(Memory* mem, uint16_t address, uint8_t value);
static void MBC5(Memory* mem, uint16_t address, uint8_t value);
static void MBC5_RAM(Memory* mem, uint16_t address, uint8_t value);
static void MBC5_RAM_BATTERY(Memory* mem, uint16_t address, uint8_t value);
static void MBC5_RUMBLE(Memory* mem, uint16_t address, uint8_t value);
static void MBC5_RUMBLE_RAM(Memory* mem, uint16_t address, uint8_t value);
static void MBC5_RUMBLE_RAM_BATTERY(Memory* mem, uint16_t address, uint8_t value);
static void MBC6(Memory* mem, uint16_t address, uint8_t value);
static void MBC7_SENSOR_RUMBLE_RAM_BATTERY(Memory* mem, uint16_t address, uint8_t value);
static void unimplemented(uint8_t mbcCode);

void (*MBC_MAP[])(Memory*, uint16_t, uint8_t) = {
    NULL,
    MBC1,
    MBC1_RAM,
    MBC1_RAM_BATTERY,
    NULL,
    MBC2,
    MBC2_BATTERY,
    NULL,
    ROM_RAM,
    ROM_RAM_BATTERY,
    NULL,
    MMM01,
    MMM01_RAM,
    MMM01_RAM_BATTERY,
    NULL,
    MBC3_TIMER_BATTERY,
    MBC3_TIMER_RAM_BATTERY,
    MBC3,
    MBC3_RAM,
    MBC3_RAM_BATTERY,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    MBC5,
    MBC5_RAM,
    MBC5_RAM_BATTERY,
    MBC5_RUMBLE,
    MBC5_RUMBLE_RAM,
    MBC5_RUMBLE_RAM_BATTERY,
    NULL,
    MBC6,
    NULL,
    MBC7_SENSOR_RUMBLE_RAM_BATTERY
};

void CART_mbcDispatch(Memory* mem, uint16_t address, uint8_t value) {
    uint8_t mbcCode = MEM_getByte(mem, 0x0147);
    void (*mbcFunction)(Memory*, uint16_t, uint8_t) = MBC_MAP[mbcCode];
    if (mbcFunction == NULL) {
        printf("Undefined MBC: 0x%x\n", mbcCode);
    } else {
        mbcFunction(mem, address, value);
    }
}

static void MBC1(Memory* mem, uint16_t address, uint8_t value) {
    Cartridge* cart = mem->cartridge;

    if (address <= 0x1FFF) {
        cart->RAMG = value & 0xF;
        mem->extRamEnabled = (cart->RAMG == 0xA);

    } else if (address <= 0x3FFF) {
        cart->BANK1 = value & 0x1F;
        if (cart->BANK1 == 0x00) cart->BANK1 = 0x01;
        if (cart->MODE == 0) {
            MEM_setRomBank(mem, (cart->BANK2 << 5) | cart->BANK1);
        } else {
            MEM_setRomBank(mem, cart->BANK1);
        }

    } else if (address <= 0x5FFF) {
        cart->BANK2 = value & 0x3;
        if (cart->MODE == 0) {
            MEM_setRomBank(mem, (cart->BANK2 << 5) | cart->BANK1);
        } else {
            MEM_setRamBank(mem, cart->BANK2);
        }

    } else if (address <= 0x7FFF) {
        cart->MODE = value & 0x1;

    }
}

static void MBC1_RAM(Memory* mem, uint16_t address, uint8_t value) {
    MBC1(mem, address, value);
}

static void MBC1_RAM_BATTERY(Memory* mem, uint16_t address, uint8_t value) {
    MBC1(mem, address, value);
}

static void MBC2(Memory* mem, uint16_t address, uint8_t value) {
    unimplemented(MEM_getByte(mem, 0x0147)); // TODO: MBC2
}

static void MBC2_BATTERY(Memory* mem, uint16_t address, uint8_t value) {
    unimplemented(MEM_getByte(mem, 0x0147)); // TODO: MBC2_BATTERY
}

static void ROM_RAM(Memory* mem, uint16_t address, uint8_t value) {
    unimplemented(MEM_getByte(mem, 0x0147)); // TODO: ROM_RAM
}

static void ROM_RAM_BATTERY(Memory* mem, uint16_t address, uint8_t value) {
    unimplemented(MEM_getByte(mem, 0x0147)); // TODO: ROM_RAM_BATTERY
}

static void MMM01(Memory* mem, uint16_t address, uint8_t value) {
    unimplemented(MEM_getByte(mem, 0x0147)); // TODO: MMM01
}

static void MMM01_RAM(Memory* mem, uint16_t address, uint8_t value) {
    unimplemented(MEM_getByte(mem, 0x0147)); // TODO: MMM01_RAM
}

static void MMM01_RAM_BATTERY(Memory* mem, uint16_t address, uint8_t value) {
    unimplemented(MEM_getByte(mem, 0x0147)); // TODO: MMM01_RAM_BATTERY
}

static void MBC3_TIMER_BATTERY(Memory* mem, uint16_t address, uint8_t value) {
    MBC3(mem, address, value);
}

static void MBC3_TIMER_RAM_BATTERY(Memory* mem, uint16_t address, uint8_t value) {
    MBC3(mem, address, value);
}

static void MBC3(Memory* mem, uint16_t address, uint8_t value) {
    //unimplemented(MEM_getByte(mem, 0x0147)); // TODO: MBC3
    Cartridge* cart = mem->cartridge;

    if (address <= 0x1FFF) {
        cart->RAMG = value & 0xF;
        mem->extRamEnabled = (cart->RAMG == 0xA);

    } else if (address <= 0x3FFF) {
        cart->ROMB = value & 0x7F;
        if (cart->ROMB == 0) cart->ROMB = 1;
        MEM_setRomBank(mem, cart->ROMB);

    } else if (address <= 0x5FFF) {

    }
}

static void MBC3_RAM(Memory* mem, uint16_t address, uint8_t value) {
    MBC3(mem, address, value);
}

static void MBC3_RAM_BATTERY(Memory* mem, uint16_t address, uint8_t value) {
    MBC3(mem, address, value);
}

static void MBC5(Memory* mem, uint16_t address, uint8_t value) {
    unimplemented(MEM_getByte(mem, 0x0147)); // TODO: MBC5
}

static void MBC5_RAM(Memory* mem, uint16_t address, uint8_t value) {
    unimplemented(MEM_getByte(mem, 0x0147)); // TODO: MBC5_RAM
}

static void MBC5_RAM_BATTERY(Memory* mem, uint16_t address, uint8_t value) {
    unimplemented(MEM_getByte(mem, 0x0147)); // TODO: MBC5_RAM_BATTERY
}

static void MBC5_RUMBLE(Memory* mem, uint16_t address, uint8_t value) {
    unimplemented(MEM_getByte(mem, 0x0147)); // TODO: MBC5_RUMBLE
}

static void MBC5_RUMBLE_RAM(Memory* mem, uint16_t address, uint8_t value) {
    unimplemented(MEM_getByte(mem, 0x0147)); // TODO: MBC5_RUMBLE_RAM
}

static void MBC5_RUMBLE_RAM_BATTERY(Memory* mem, uint16_t address, uint8_t value) {
    unimplemented(MEM_getByte(mem, 0x0147)); // TODO: MBC5_RUMBLE_RAM_BATTERY
}

static void MBC6(Memory* mem, uint16_t address, uint8_t value) {
    unimplemented(MEM_getByte(mem, 0x0147)); // TODO: MBC6
}

static void MBC7_SENSOR_RUMBLE_RAM_BATTERY(Memory* mem, uint16_t address, uint8_t value) {
    unimplemented(MEM_getByte(mem, 0x0147)); // TODO: MBC7_SENSOR_RUMBLE_RAM_BATTERY
}

static void unimplemented(uint8_t mbcCode) {
    fprintf(stderr, "Unimplemented MBC: 0x%x\n", mbcCode);
    exit(1);
}