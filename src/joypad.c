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

void JOY_update(Joypad* joy, Memory* mem) {
    MEM_setByte(mem, REG_JOYP, MEM_getByte(mem, REG_JOYP) | 0xCF);

    if (!getBit(MEM_getByte(mem, REG_JOYP), 5)) {
        // Action buttons
        if (joy->a) MEM_setByte(mem, REG_JOYP, setBit(MEM_getByte(mem, REG_JOYP), 0, 0));
        if (joy->b) MEM_setByte(mem, REG_JOYP, setBit(MEM_getByte(mem, REG_JOYP), 1, 0));
        if (joy->select) MEM_setByte(mem, REG_JOYP, setBit(MEM_getByte(mem, REG_JOYP), 2, 0));
        if (joy->start) MEM_setByte(mem, REG_JOYP, setBit(MEM_getByte(mem, REG_JOYP), 3, 0));
    }
    if (!getBit(MEM_getByte(mem, REG_JOYP), 4)) {
        // Direction buttons
        if (joy->right) MEM_setByte(mem, REG_JOYP, setBit(MEM_getByte(mem, REG_JOYP), 0, 0));
        if (joy->left) MEM_setByte(mem, REG_JOYP, setBit(MEM_getByte(mem, REG_JOYP), 1, 0));
        if (joy->up) MEM_setByte(mem, REG_JOYP, setBit(MEM_getByte(mem, REG_JOYP), 2, 0));
        if (joy->down) MEM_setByte(mem, REG_JOYP, setBit(MEM_getByte(mem, REG_JOYP), 3, 0));
    }

    if ((MEM_getByte(mem, REG_JOYP) & 0xF) != 0xF) {
        // Request joypad interrupt
        MEM_setByte(mem, REG_IF, MEM_getByte(mem, REG_IF) | 0x10);
    }
}