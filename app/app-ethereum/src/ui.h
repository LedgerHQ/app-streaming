#pragma once

#include <stdbool.h>

#include "uint256.h"
#include "ux/ux.h"

bool ui_get_pubkey_validation(void);
void ui_set_pubkey_address(char *address);

bool ui_sign_tx_validation(void);
void ui_set_tx_address(uint64_t chain_id, uint8_t *to);
void ui_set_tx_network_name(uint64_t chain_id);
void ui_set_tx_fees(uint64_t chain_id, const uint256_t *gas_price, const uint256_t *gas_limit);
void ui_set_tx_amount(uint64_t chain_id, const uint256_t *amount);
bool ui_eip712(struct ux_item_s *items, size_t count);
