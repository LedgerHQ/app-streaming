#pragma once

#include "api/uint256.h"

#define INT256_LENGTH 32

void readu256BE(uint8_t *buffer, uint256_t *target);
void mul256(uint256_t *number1, uint256_t *number2, uint256_t *target);
