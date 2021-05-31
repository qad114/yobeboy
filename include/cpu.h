#ifndef CPU_H
#define CPU_H

typedef struct CPU CPU;

#include <stdint.h>
#include "common/endianness.h"
#include "gpu.h"
#include "joypad.h"
#include "memory.h"
#include "timer.h"

struct CPU {
    // Opcodes can be 8- or 16-bit - we will use an 8-bit variable and decode the next bits when necessary
    uint8_t opcode;

    // Registers
    union {
        #if ENDIANNESS == LITTLE_E
        struct { uint8_t F, A; };
        #else
        struct { uint8_t A, F; };
        #endif
        uint16_t AF;
    };

    union {
        #if ENDIANNESS == LITTLE_E
        struct { uint8_t C, B; };
        #else
        struct { uint8_t B, C; };
        #endif
        uint16_t BC;
    };

    union {
        #if ENDIANNESS == LITTLE_E
        struct { uint8_t E, D; };
        #else
        struct { uint8_t D, E; };
        #endif
        uint16_t DE;
    };

    union {
        #if ENDIANNESS == LITTLE_E
        struct { uint8_t L, H; };
        #else
        struct { uint8_t H, L; };
        #endif
        uint16_t HL;
    };

    uint16_t SP;
    uint16_t PC;

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