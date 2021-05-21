#include "constants.h"
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

void JOY_update() {
    // TODO
}