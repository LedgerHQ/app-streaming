#include <string.h>

#include "crypto.h"

#include "api/uint256.h"

#define UPPER_P(x) x->elements[0]
#define LOWER_P(x) x->elements[1]
#define UPPER(x)   x.elements[0]
#define LOWER(x)   x.elements[1]

static uint64_t readUint64BE(uint8_t *buffer)
{
    return (((uint64_t)buffer[0]) << 56) | (((uint64_t)buffer[1]) << 48) |
           (((uint64_t)buffer[2]) << 40) | (((uint64_t)buffer[3]) << 32) |
           (((uint64_t)buffer[4]) << 24) | (((uint64_t)buffer[5]) << 16) |
           (((uint64_t)buffer[6]) << 8) | (((uint64_t)buffer[7]));
}

static void readu128BE(uint8_t *buffer, uint128_t *target)
{
    UPPER_P(target) = readUint64BE(buffer);
    LOWER_P(target) = readUint64BE(buffer + 8);
}

void readu256BE(uint8_t *buffer, uint256_t *target)
{
    readu128BE(buffer, &UPPER_P(target));
    readu128BE(buffer + 16, &LOWER_P(target));
}

static void write_u64_be(uint8_t *buffer, uint64_t value)
{
    buffer[0] = ((value >> 56) & 0xff);
    buffer[1] = ((value >> 48) & 0xff);
    buffer[2] = ((value >> 40) & 0xff);
    buffer[3] = ((value >> 32) & 0xff);
    buffer[4] = ((value >> 24) & 0xff);
    buffer[5] = ((value >> 16) & 0xff);
    buffer[6] = ((value >> 8) & 0xff);
    buffer[7] = (value & 0xff);
}

static void read_u64_be(uint8_t *in, uint64_t *out)
{
    uint8_t *out_ptr = (uint8_t *)out;
    *out_ptr++ = in[7];
    *out_ptr++ = in[6];
    *out_ptr++ = in[5];
    *out_ptr++ = in[4];
    *out_ptr++ = in[3];
    *out_ptr++ = in[2];
    *out_ptr++ = in[1];
    *out_ptr = in[0];
}

void mul256(uint256_t *number1, uint256_t *number2, uint256_t *target)
{
    uint8_t num1[INT256_LENGTH], num2[INT256_LENGTH], result[INT256_LENGTH * 2];
    memset(&result, 0, sizeof(result));
    for (uint8_t i = 0; i < 4; i++) {
        write_u64_be(num1 + i * sizeof(uint64_t), number1->elements[i / 2].elements[i % 2]);
        write_u64_be(num2 + i * sizeof(uint64_t), number2->elements[i / 2].elements[i % 2]);
    }
    mult(result, num1, num2, sizeof(num1));
    for (uint8_t i = 0; i < 4; i++) {
        read_u64_be(result + 32 + i * sizeof(uint64_t), &target->elements[i / 2].elements[i % 2]);
    }
}
