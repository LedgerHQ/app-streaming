#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_SIG_SIZE 72

bool encode_bytes(const uint8_t *bytes, const size_t size, uint8_t **out, size_t *p_out_size);
size_t sign_tx(const uint32_t *path,
               const size_t path_count,
               const uint8_t *tx,
               const size_t tx_size,
               uint8_t *buffer,
               const size_t max_size);
size_t rlp_encode_tx(const char *address,
                     const uint8_t *amount,
                     const size_t amount_size,
                     const uint8_t *fees,
                     const size_t fees_size,
                     uint8_t *buffer,
                     const size_t buf_size);
size_t rlp_buffer_to_list(uint8_t *buffer, const size_t size, const size_t max_size);
size_t eth_create_tx(const uint32_t *path,
                     const size_t path_count,
                     const char *address,
                     const uint8_t *amount,
                     const size_t amount_size,
                     const uint8_t *fees,
                     const size_t fees_size,
                     uint8_t *bytes,
                     const size_t max_size);
