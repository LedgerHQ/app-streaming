#pragma once

#include <stdbool.h>

bool ui_get_pubkey_validation(void);
void ui_set_pubkey_address(char *address);

bool ui_sign_tx_validation(void);
void ui_set_tx_address(uint64_t chain_id, char *to);
void ui_set_tx_network_name(uint64_t chain_id);
void ui_set_tx_fees(uint64_t chain_id, const txInt256_t *gas_price, const txInt256_t *gas_limit);
void ui_set_tx_amount(uint64_t chain_id, const txInt256_t *amount);
