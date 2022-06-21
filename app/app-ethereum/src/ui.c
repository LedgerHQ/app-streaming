#include <stdio.h>
#include <stdlib.h>

#include "app-ethereum.h"
#include "sdk.h"
#include "ux/glyphs.h"
#include "ux/ux.h"

#include "ui.h"

static char g_ux_get_pubkey_full_address[64];

static struct ux_item_s ux_get_pubkey[] = {
    { &C_icon_eye, "Verify", "address", UX_ACTION_NONE },
    { NULL, "Address:", g_ux_get_pubkey_full_address, UX_ACTION_NONE },
    { &C_icon_validate_14, "Approve", NULL, UX_ACTION_VALIDATE },
    { &C_icon_crossmark, "Reject", NULL, UX_ACTION_REJECT },
};

static char g_ux_sign_tx_amount[79];
static char g_ux_sign_tx_full_address[41];
static char g_ux_sign_tx_max_fee[50];
static char g_ux_sign_tx_network_name[79];

static struct ux_item_s ux_sign_tx[] = {
    { &C_icon_eye, "Review", "transaction", UX_ACTION_NONE },
    { NULL, "Amount", g_ux_sign_tx_amount, UX_ACTION_NONE },
    { NULL, "Address", g_ux_sign_tx_full_address, UX_ACTION_NONE },
    { NULL, "Max Fees", g_ux_sign_tx_max_fee, UX_ACTION_NONE },
    { NULL, "Network", g_ux_sign_tx_network_name, UX_ACTION_NONE },
    { &C_icon_validate_14, "Accept", "and send", UX_ACTION_VALIDATE },
    { &C_icon_crossmark, "Reject", NULL, UX_ACTION_REJECT },
};

void ui_set_pubkey_address(char *address)
{
    snprintf(g_ux_get_pubkey_full_address, sizeof(g_ux_get_pubkey_full_address), "0x%.*s", 40,
             address);
}

/* return true if the user approved the tx, false otherwise */
bool ui_get_pubkey_validation(void)
{
    bool app_loading = app_loading_stop();

    bool validated = ux_validate(ux_get_pubkey, ARRAY_SIZE(ux_get_pubkey));

    if (app_loading) {
        app_loading_start(NULL);
    } else {
        ux_idle();
    }

    return validated;
}

void ui_set_tx_address(uint64_t chain_id, uint8_t *to)
{
    getEthAddressStringFromBinary(to, g_ux_sign_tx_full_address, chain_id);
}

void ui_set_tx_network_name(uint64_t chain_id)
{
    set_network_name(chain_id, g_ux_sign_tx_network_name, sizeof(g_ux_sign_tx_network_name));
}

void ui_set_tx_fees(uint64_t chain_id, const uint256_t *gas_price, const uint256_t *gas_limit)
{
    compute_fees(chain_id, gas_price, gas_limit, g_ux_sign_tx_max_fee,
                 sizeof(g_ux_sign_tx_max_fee));
}

void ui_set_tx_amount(uint64_t chain_id, const uint256_t *amount)
{
    compute_amount(chain_id, amount, g_ux_sign_tx_amount, sizeof(g_ux_sign_tx_amount));
}

/* return true if the user approved the tx, false otherwise */
bool ui_sign_tx_validation(void)
{
    bool app_loading = app_loading_stop();

    bool validated = ux_validate(ux_sign_tx, ARRAY_SIZE(ux_sign_tx));

    if (app_loading) {
        app_loading_start("Signing transaction...");
    } else {
        ux_idle();
    }

    return validated;
}

/* return true if the user approved the tx, false otherwise */
bool ui_eip712(struct ux_item_s *items, size_t count)
{
    bool app_loading = app_loading_stop();

    bool validated = ux_validate(items, count);

    if (app_loading) {
        app_loading_start("Signing EIP712...");
    } else {
        ux_idle();
    }

    return validated;
}
