#pragma once

#include <stdint.h>

#define INT256_LENGTH 32

typedef struct uint128_t {
    uint64_t elements[2];
} uint128_t;

typedef struct uint256_t {
    uint128_t elements[2];
} uint256_t;

void readu256BE(uint8_t *buffer, uint256_t *target);
void mul256(uint256_t *number1, uint256_t *number2, uint256_t *target);
