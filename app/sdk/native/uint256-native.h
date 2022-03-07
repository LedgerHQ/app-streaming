#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "api/uint256.h"

void readu128BE(uint8_t *buffer, uint128_t *target);
void write_u64_be(uint8_t *buffer, uint64_t value);
bool zero128(uint128_t *number);
bool zero256(uint256_t *number);
void copy128(uint128_t *target, uint128_t *number);
void copy256(uint256_t *target, uint256_t *number);
void clear128(uint128_t *target);
void clear256(uint256_t *target);
void shiftl128(uint128_t *number, uint32_t value, uint128_t *target);
void shiftr128(uint128_t *number, uint32_t value, uint128_t *target);
void shiftl256(uint256_t *number, uint32_t value, uint256_t *target);
void shiftr256(uint256_t *number, uint32_t value, uint256_t *target);
uint32_t bits128(uint128_t *number);
uint32_t bits256(uint256_t *number);
bool equal128(uint128_t *number1, uint128_t *number2);
bool equal256(uint256_t *number1, uint256_t *number2);
bool gt128(uint128_t *number1, uint128_t *number2);
bool gt256(uint256_t *number1, uint256_t *number2);
bool gte128(uint128_t *number1, uint128_t *number2);
bool gte256(uint256_t *number1, uint256_t *number2);
void add128(uint128_t *number1, uint128_t *number2, uint128_t *target);
void add256(uint256_t *number1, uint256_t *number2, uint256_t *target);
void minus128(uint128_t *number1, uint128_t *number2, uint128_t *target);
void minus256(uint256_t *number1, uint256_t *number2, uint256_t *target);
void or128(uint128_t *number1, uint128_t *number2, uint128_t *target);
void or256(uint256_t *number1, uint256_t *number2, uint256_t *target);
void mul128(uint128_t *number1, uint128_t *number2, uint128_t *target);
void divmod128(uint128_t *l, uint128_t *r, uint128_t *div, uint128_t *mod);
void divmod256(uint256_t *l, uint256_t *r, uint256_t *div, uint256_t *mod);
bool sys_tostring256(const uint256_t *number, const unsigned int base, char *out, size_t len);
void reverseString(char *str, uint32_t length);
