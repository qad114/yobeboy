#ifndef COMMON__BITWISE_H
#define COMMON__BITWISE_H

#include <stdint.h>

static inline uint8_t getBit(uint8_t byte, int bit) {
    return (byte >> bit) & 1;
}

static inline uint8_t setBit(uint8_t byte, int bit, int value) {
    return value ? (byte | (1 << bit)) : (byte & ~((uint8_t) (1 << bit)));
}

#endif