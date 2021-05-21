#ifndef CPU_H
#define CPU_H

typedef struct CPU CPU;

#include <stdint.h>
#include "gpu.h"
#include "joypad.h"
#include "memory.h"
#include "timer.h"

struct CPU {
    // Opcodes can be 8- or 16-bit - we will use an 8-bit variable and decode the next bits when necessary
    uint8_t opcode;

    // Registers - AF, BC, DE and HL can be used as one 16-bit register
    uint8_t A, B, C, D, E, F, H, L;
    uint16_t SP, PC;

    // Interrupt Master Enable flag
    int IME;
};

// Flag getters
int CPU_getFlagZ(CPU* cpu);
int CPU_getFlagN(CPU* cpu);
int CPU_getFlagH(CPU* cpu);
int CPU_getFlagC(CPU* cpu);

// Flag setters
void CPU_setFlagZ(CPU* cpu, int value);
void CPU_setFlagN(CPU* cpu, int value);
void CPU_setFlagH(CPU* cpu, int value);
void CPU_setFlagC(CPU* cpu, int value);

void CPU_init(CPU* cpu);
int CPU_emulateCycle(CPU* cpu, GPU* gpu, Memory* mem, Timer* timer, Joypad* joy);

#endif