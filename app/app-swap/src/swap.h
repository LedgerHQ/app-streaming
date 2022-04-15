#pragma once

#include <stdbool.h>
#include <stdlib.h>

#include "api/uint256.h"
#include "crypto.h"
#include "sdk.h"

#include "currency_config.h"
#include "message.pb.h"

typedef struct swap_ctx_s {
    char device_id_swap[11];
    uint8_t device_id_sell[32];
} swap_ctx_t;

const char *sign(const uint32_t *path,
                 const size_t path_count,
                 const uint8_t *hash,
                 uint8_t *bytes,
                 const size_t max_size,
                 uint16_t *size);

bool verify_partner(const Partner *partner);
bool ui_sign_tx_validation(const char *send_amount, const char *get_amount, const char *fees);

const char *handle_init_swap(const RequestInitSwap *req,
                             ResponseInitSwap *response,
                             swap_ctx_t *ctx);
const char *handle_init_sell(const RequestInitSell *req,
                             ResponseInitSell *response,
                             swap_ctx_t *ctx);
const char *handle_swap(const RequestSwap *req, ResponseSwap *response, swap_ctx_t *ctx);
const char *handle_sell(const RequestSell *req, ResponseSell *response, swap_ctx_t *ctx);

const swap_config_t *get_config(const char *currency);
