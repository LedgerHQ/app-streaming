#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "app-ethereum.h"
#include "uint256.h"

struct tx_s {
    txInt256_t nonce;
    txInt256_t gas_price;
    txInt256_t gas_limit;
    uint8_t *to;
    txInt256_t value;
    uint8_t *data;
    size_t data_size;
    uint64_t chain_id;
};

bool rlp_decode_list(const uint8_t *data, const size_t size, struct tx_s *tx);
