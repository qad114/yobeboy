#ifndef COMMON__BITWISE_H
#define COMMON__BITWISE_H

#include <stdint.h>

/*// Return one bit of a byte as a uint8_t
//uint8_t getBit(uint8_t byte, int bit);
#define GET_BIT(byte, bitIndex) (((byte) >> (bitIndex)) & 1)

static inline uint8_t get_bit(uint8_t byte, uint8_t bit)

// Set one bit of a byte and return the changed byte
uint8_t setBit(uint8_t byte, int bit, int value);

// Shift a byte right (by a positive size) or left (by a negative size)
uint8_t shiftByte(uint8_t byte, int size);*/

static inline uint8_t getBit(uint8_t byte, int bit) {
    return (byte >> bit) & 1;
}

static inline uint8_t setBit(uint8_t byte, int bit, int value) {
    return value ? (byte | (1 << bit)) : (byte & ~((uint8_t) (1 << bit)));
}

#endif