#include <stdio.h>

#include "common/bitwise.h"
#include "constants.h"
#include "cpu.h"
#include "memory.h"
#include "timer.h"

static void updateDiv(Memory* mem, Timer* timer);
static void updateTima(Memory* mem, Timer* timer);

void TIMER_init(Timer* timer) {
    timer->divCounter = 0;
}

void TIMER_destroy(Timer* timer) {
    free(timer);
    timer = NULL;
}

// Update the timer and divider registers
void TIMER_update(CPU* cpu, Memory* mem, Timer* timer) {
    uint8_t* TIMA = &(mem->logicalMemory[REG_TIMA]);
    uint8_t TAC = mem->logicalMemory[REG_TAC];

    // Update DIV every 256 machine cycles
    if (timer->divCounter == 64) {//256) {
        timer->divCounter = 0;
        updateDiv(mem, timer);
    } else {
        ++(timer->divCounter);
    }

    // Update TIMA
    if (!getBit(TAC, 2)) {
        timer->timaCounter = 0;
        *TIMA = 0;
        return;
    }

    int interval = 0;
    switch (TAC & 0x3) {
        case 0: interval = 256; break;
        case 1: interval = 4; break;
        case 2: interval = 16; break;
        case 3: interval = 64; break;
    }

    //++(timer->timaCounter);
    if (timer->timaCounter >= interval) {
        timer->timaCounter = (timer->timaCounter - interval + 1);
        updateTima(mem, timer);
    } else {
        ++(timer->timaCounter);
    }
}

static void updateDiv(Memory* mem, Timer* timer) {
    uint8_t* DIV = &(mem->logicalMemory[REG_DIV]);
    if (*DIV == 0) {
        timer->divCounter = 0;
    }
    ++*DIV;
}

static void updateTima(Memory* mem, Timer* timer) {
    uint8_t* TIMA = &(mem->logicalMemory[REG_TIMA]);
    if (*TIMA == 0xFF) {
        *TIMA = mem->logicalMemory[REG_TMA];
        mem->logicalMemory[REG_IF] |= 0x4;
    } else {
        ++*TIMA;
    }
}