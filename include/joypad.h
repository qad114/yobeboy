#ifndef JOYPAD_H
#define JOYPAD_H

typedef struct Joypad Joypad;

struct Joypad {
    int a, b, start, select, up, down, left, right;
};

void JOY_init(Joypad* joy);
void JOY_update(Joypad* joy, Memory* mem);

#endif