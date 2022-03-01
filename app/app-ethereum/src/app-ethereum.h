#pragma once

#include <stdbool.h>
#include <stdlib.h>

#include "crypto.h"
#include "sdk.h"
#include "uint256.h"

#include "message.pb.h"

typedef struct txInt256_s {
    uint8_t value[INT256_LENGTH];
    uint8_t length;
} txInt256_t;

const char *handle_get_pubkey(const RequestGetPubKey *req, ResponseGetPubKey *response);
const char *handle_sign_tx(const RequestSignTx *req, ResponseSignTx *response);

void getEthAddressStringFromBinary(uint8_t *hash_address, char *out, uint64_t chain_id);
void getEthAddressStringFromKey(cx_ecfp_public_key_t *publicKey, char *out, uint64_t chainId);
void set_network_name(const uint64_t chain_id, char *name, size_t size);
void compute_fees(uint64_t chain_id, const txInt256_t *gas_price, const txInt256_t *gas_limit, char *buffer, const size_t size);
void compute_amount(uint64_t chain_id, const txInt256_t *amount, char *out_buffer, size_t out_buffer_size);
