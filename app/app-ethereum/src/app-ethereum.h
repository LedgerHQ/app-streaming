#pragma once

#include <stdbool.h>
#include <stdlib.h>

#include "crypto.h"
#include "sdk.h"
#include "uint256.h"

#include "message.pb.h"

const char *sign(const uint32_t *path,
                 const size_t path_count,
                 const uint8_t *hash,
                 uint8_t *bytes,
                 const size_t max_size,
                 uint16_t *size);

const char *handle_get_pubkey(const RequestGetPubKey *req, ResponseGetPubKey *response);
const char *handle_sign_tx(const RequestSignTx *req, ResponseSignTx *response);
const char *handle_sign_message(const RequestSignMsg *req, ResponseSignMsg *response);

void getEthAddressStringFromBinary(uint8_t *hash_address, char *out, uint64_t chain_id);
void getEthAddressStringFromKey(cx_ecfp_public_key_t *publicKey, char *out, uint64_t chainId);
void set_network_name(const uint64_t chain_id, char *name, size_t size);
void compute_fees(uint64_t chain_id,
                  const uint256_t *gas_price,
                  const uint256_t *gas_limit,
                  char *buffer,
                  const size_t size);
void compute_amount(uint64_t chain_id,
                    const uint256_t *amount,
                    char *out_buffer,
                    size_t out_buffer_size);
