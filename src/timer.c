#include <stdio.h>

#include "constants.h"
#include "cpu.h"
#include "memory.h"
#include "timer.h"

// Update the timer and divider registers
void TIMER_update(CPU* cpu, Memory* mem, Timer* timer) {
    // Update DIV every 256 machine cycles
    if (timer->divCounter == 64) {//256) {
        timer->divCounter = 0;
        updateDiv(cpu, mem, timer);
    } else {
        ++(timer->divCounter);
    }

    // Update TIMA
    /*if (!((MEM_getByte(mem, REG_TAC) & 0b100) >> 2)) {
        timer->timaCounter = 0;
        MEM_setByte(mem, REG_TIMA, 0);
        return;
    }

    int interval = 0;
    switch (MEM_getByte(mem, REG_TAC) & 0b11) {
        case 0: interval = 256; break;
        case 1: interval = 4; break;
        case 2: interval = 16; break;
        case 3: interval = 64; break;
    }

    //++(timer->timaCounter);
    if (timer->timaCounter >= interval) {
        timer->timaCounter = (timer->timaCounter - interval + 1);
        updateTima(cpu, mem, timer);
    } else {
        ++(timer->timaCounter);
    }*/
}

void updateDiv(CPU* cpu, Memory* mem, Timer* timer) {
    uint8_t DIV = MEM_getByte(mem, REG_DIV);
    if (DIV == 0) {
        timer->divCounter = 0;
    }
    MEM_forceSetByte(mem, REG_DIV, DIV + 1);
}

void updateTima(CPU* cpu, Memory* mem, Timer* timer) {
    uint8_t TIMA = MEM_getByte(mem, REG_TIMA);
    if (TIMA == 0xFF) {
        MEM_setByte(mem, REG_TIMA, MEM_getByte(mem, REG_TMA));
        MEM_setByte(mem, REG_IF, MEM_getByte(mem, REG_IF) | 0b100);
    } else {
        MEM_forceSetByte(mem, REG_TIMA, TIMA + 1);
    }
}