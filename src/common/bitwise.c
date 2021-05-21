#include "common/bitwise.h"

uint8_t getBit(uint8_t byte, int bit) {
    //return (byte & (1 << bit)) >> bit;
    return (byte >> bit) & 1;
}

uint8_t setBit(uint8_t byte, int bit, int value) {
    return value ? (byte | (1 << bit)) : (byte & ~((uint8_t) (1 << bit)));
}

uint8_t shiftByte(uint8_t byte, int size) {
    return (size < 0) ? (byte << (-1 * size)) : (size > 0) ? (byte >> size) : byte;
}