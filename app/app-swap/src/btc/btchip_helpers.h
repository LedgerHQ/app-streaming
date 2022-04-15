#pragma once

#include <stddef.h>
#include <stdint.h>

void btchip_public_key_hash160(const uint8_t *in, const size_t inlen, uint8_t *out);
void btchip_compute_checksum(uint8_t *in, size_t inlen, uint8_t *output);
size_t btchip_public_key_to_encoded_base58(const uint8_t *in,
                                           const size_t inlen,
                                           uint8_t *out,
                                           const size_t outlen,
                                           const unsigned short version,
                                           const uint8_t alreadyHashed);
size_t btchip_convert_hex_amount_to_displayable(const uint8_t *amount, uint8_t *out);
