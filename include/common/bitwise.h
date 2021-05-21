#ifndef COMMON__BITWISE_H
#define COMMON__BITWISE_H

#include <stdint.h>

// Return one bit of a byte as a uint8t
uint8_t getBit(uint8_t byte, int bit);

// Set one bit of a byte and return the changed byte
uint8_t setBit(uint8_t byte, int bit, int value);

// Shift a byte right (by a positive size) or left (by a negative size)
uint8_t shiftByte(uint8_t byte, int size);

#endif