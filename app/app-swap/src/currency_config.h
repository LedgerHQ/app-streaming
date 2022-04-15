#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef bool (*validate_address_t)(const char *address,
                                   const uint32_t *path,
                                   const size_t path_count);
typedef bool (*get_printable_amount_t)(const uint8_t *bytes,
                                       const size_t size,
                                       char *out,
                                       const size_t out_size);
typedef size_t (*create_tx_t)(const uint32_t *path,
                              const size_t path_count,
                              const char *address,
                              const uint8_t *amount,
                              const size_t amount_size,
                              const uint8_t *fees,
                              const size_t fees_size,
                              uint8_t *bytes,
                              const size_t max_size);

typedef struct swap_config_s {
    validate_address_t validate_address;
    get_printable_amount_t get_printable_amount;
    create_tx_t create_tx;
} swap_config_t;
