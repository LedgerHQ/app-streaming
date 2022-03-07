#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "api/uint256.h"
#include "app-ethereum.h"

struct tx_s {
    uint256_t nonce;
    uint256_t gas_price;
    uint256_t gas_limit;
    uint8_t *to;
    uint256_t value;
    uint8_t *data;
    size_t data_size;
    uint64_t chain_id;
};

bool rlp_decode_list(const uint8_t *data, const size_t size, struct tx_s *tx);
