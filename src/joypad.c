#include <stdlib.h>

#include "common/bitwise.h"
#include "constants.h"
#include "memory.h"
#include "joypad.h"

void JOY_init(Joypad* joy) {
    joy->a = 0;
    joy->b = 0;
    joy->start = 0;
    joy->select = 0;
    joy->up = 0;
    joy->down = 0;
    joy->left = 0;
    joy->right = 0;
}

void JOY_destroy(Joypad* joy) {
    free(joy);
    joy = NULL;
}

void JOY_update(Joypad* joy, Memory* mem) {
    uint8_t* JOYP = &(mem->logicalMemory[REG_JOYP]);
    *JOYP |= 0xCF;

    if (!getBit(*JOYP, 5)) {
        // Action buttons
        if (joy->a) *JOYP = setBit(*JOYP, 0, 0);
        if (joy->b) *JOYP = setBit(*JOYP, 1, 0);
        if (joy->select) *JOYP = setBit(*JOYP, 2, 0);
        if (joy->start) *JOYP = setBit(*JOYP, 3, 0);
    }
    if (!getBit(*JOYP, 4)) {
        // Direction buttons
        if (joy->right) *JOYP = setBit(*JOYP, 0, 0);
        if (joy->left) *JOYP = setBit(*JOYP, 1, 0);
        if (joy->up) *JOYP = setBit(*JOYP, 2, 0);
        if (joy->down) *JOYP = setBit(*JOYP, 3, 0);
    }

    if ((*JOYP & 0xF) != 0xF) {
        // Request joypad interrupt
        mem->logicalMemory[REG_IF] |= 0x10;
    }
}