#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../currency_config.h"
#include "crypto.h"

void getEthAddressStringFromKey(cx_ecfp_public_key_t *publicKey, char *out, uint64_t chainId);

bool amountToString(const uint8_t *amount,
                    uint8_t amount_size,
                    uint8_t decimals,
                    const char *ticker,
                    char *out_buffer,
                    size_t out_buffer_size);

extern const swap_config_t eth_config;
