#ifndef TIMER_H
#define TIMER_H

typedef struct Timer Timer;

#include <stdlib.h>

#include "cpu.h"
#include "memory.h"

struct Timer {
    int divCounter;
    int timaCounter;
};

void TIMER_init(Timer* timer);
void TIMER_destroy(Timer* timer);
void TIMER_update(CPU* cpu, Memory* mem, Timer* timer);

#endif