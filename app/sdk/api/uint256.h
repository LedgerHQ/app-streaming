#pragma once

#include <stdint.h>

typedef struct uint128_t {
    uint64_t elements[2];
} uint128_t;

typedef struct uint256_t {
    uint128_t elements[2];
} uint256_t;
