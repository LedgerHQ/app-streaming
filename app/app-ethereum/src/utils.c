#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app-ethereum.h"

static const char HEXDIGITS[] = "0123456789abcdef";

#define ADDRESS_SIZE 40
#define INT256_LENGTH 32

static void hexdump(uint8_t *src, uint8_t *dst, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        uint8_t digit = src[i];
        dst[2 * i] = HEXDIGITS[(digit >> 4) & 0x0f];
        dst[2 * i + 1] = HEXDIGITS[digit & 0x0f];
    }
}

static void checksum(const uint8_t *hash_checksum, const uint8_t *hash_address, char *out)
{
    for (size_t i = 0; i < ADDRESS_SIZE; i++) {
        uint8_t digit = hash_address[i / 2];
        if ((i % 2) == 0) {
            digit = (digit >> 4) & 0x0f;
        } else {
            digit = digit & 0x0f;
        }
        if (digit < 10) {
            out[i] = HEXDIGITS[digit];
        } else {
            int v = (hash_checksum[i / 2] >> (4 * (1 - i % 2))) & 0x0f;
            if (v >= 8) {
                out[i] = HEXDIGITS[digit] - 'a' + 'A';
            } else {
                out[i] = HEXDIGITS[digit];
            }
        }
    }

    out[ADDRESS_SIZE] = '\0';
}

static void getEthAddressStringFromBinary(uint8_t *hash_address, char *out, uint64_t chain_id)
{
    uint8_t hash_checksum[INT256_LENGTH];

    if (chain_id == 30 || chain_id == 31) {
        /* EIP-1191 */
        uint8_t tmp[20 + 2 + ADDRESS_SIZE + 1];
        size_t offset = snprintf((char *)tmp, sizeof(tmp), "%" PRIu64 "0x", chain_id);
        hexdump(hash_address, &tmp[offset], 20);
        sha3_256(tmp, offset + ADDRESS_SIZE, hash_checksum);
    } else {
        uint8_t tmp[ADDRESS_SIZE];

        hexdump(hash_address, tmp, 20);
        sha3_256(tmp, ADDRESS_SIZE, hash_checksum);
    }

    checksum(hash_checksum, hash_address, out);
}

void getEthAddressStringFromKey(cx_ecfp_public_key_t *publicKey, char *out, uint64_t chain_id)
{
    uint8_t hashAddress[INT256_LENGTH];
    sha3_256(publicKey->W + 1, 64, hashAddress);
    getEthAddressStringFromBinary(hashAddress + 12, out, chain_id);
}
