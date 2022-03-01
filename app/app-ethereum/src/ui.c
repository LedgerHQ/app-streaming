#include <stdio.h>
#include <stdlib.h>

#include "app-ethereum.h"
#include "sdk.h"
#include "ux/glyphs.h"
#include "ux/ux.h"

#include "ui.h"

static int validated;
static char g_ux_get_pubkey_full_address[64];

UX_STEP_NOCB(ux_get_pubkey_1_step, pnn, {&C_icon_eye, "Verify", "address"});
UX_STEP_NOCB(ux_get_pubkey_2_step, nn, {"Address:", g_ux_get_pubkey_full_address});
UX_STEP_CB(ux_get_pubkey_3_step, pb, validated = 1, {&C_icon_validate_14, "Approve"});
UX_STEP_CB(ux_get_pubkey_4_step, pb, validated = -1, {&C_icon_crossmark, "Reject"});

UX_FLOW(ux_get_pubkey_flow,
        &ux_get_pubkey_1_step,
        &ux_get_pubkey_2_step,
        &ux_get_pubkey_3_step,
        &ux_get_pubkey_4_step);

static char g_ux_sign_tx_amount[79];
static char g_ux_sign_tx_full_address[41];
static char g_ux_sign_tx_max_fee[50];
static char g_ux_sign_tx_network_name[79];

UX_STEP_NOCB(ux_sign_tx_1_step, pnn, {&C_icon_eye, "Review", "transaction"});
UX_STEP_NOCB(ux_sign_tx_2_step, nn, {"Amount", g_ux_sign_tx_amount});
UX_STEP_NOCB(ux_sign_tx_3_step, nn, {"Address", g_ux_sign_tx_full_address});
UX_STEP_NOCB(ux_sign_tx_4_step, nn, {"Max Fees", g_ux_sign_tx_max_fee});
UX_STEP_NOCB(ux_sign_tx_5_step, nn, {"Network", g_ux_sign_tx_network_name});
UX_STEP_CB(ux_sign_tx_6_step, pbb, validated = 1, {&C_icon_validate_14, "Accept", "and send"});
UX_STEP_CB(ux_sign_tx_7_step, pb, validated = -1, {&C_icon_crossmark, "Reject"});

UX_FLOW(ux_sign_tx_flow,
        &ux_sign_tx_1_step,
        &ux_sign_tx_2_step,
        &ux_sign_tx_3_step,
        &ux_sign_tx_4_step,
        &ux_sign_tx_5_step,
        &ux_sign_tx_6_step,
        &ux_sign_tx_7_step);

static void ui_button_helper(int button)
{
    unsigned int button_mask = button | BUTTON_EVT_RELEASED;

    if (G_ux.stack[0].button_push_callback != NULL) {
        G_ux.stack[0].button_push_callback(button_mask, 0);
    }
}

static void ui_init(const ux_flow_step_t * const *steps)
{
    memset(&G_ux, 0, sizeof(G_ux));
    ux_stack_push();
    ux_flow_init(0, steps, NULL);
}

void ui_set_pubkey_address(char *address)
{
    snprintf(g_ux_get_pubkey_full_address, sizeof(g_ux_get_pubkey_full_address),
             "0x%.*s", 40, address);
}

/* return true if the user approved the tx, false otherwise */
bool ui_get_pubkey_validation(void)
{
    validated = 0;

    bool app_loading = app_loading_stop();

    ui_init(ux_get_pubkey_flow);

    while (validated == 0) {
        int button = wait_button();
        ui_button_helper(button);
    }

    if (app_loading) {
        app_loading_start();
    } else {
        ux_idle();
    }

    return (validated == 1);
}

void ui_set_tx_address(uint64_t chain_id, char *to)
{
    getEthAddressStringFromBinary(to, g_ux_sign_tx_full_address, chain_id);
}

void ui_set_tx_network_name(uint64_t chain_id)
{
    set_network_name(chain_id, g_ux_sign_tx_network_name, sizeof(g_ux_sign_tx_network_name));
}

void ui_set_tx_fees(uint64_t chain_id, const txInt256_t *gas_price, const txInt256_t *gas_limit)
{
    compute_fees(chain_id, gas_price, gas_limit, g_ux_sign_tx_max_fee, sizeof(g_ux_sign_tx_max_fee));
}

void ui_set_tx_amount(uint64_t chain_id, const txInt256_t *amount)
{
    compute_amount(chain_id, amount, g_ux_sign_tx_amount, sizeof(g_ux_sign_tx_amount));
}

/* return true if the user approved the tx, false otherwise */
bool ui_sign_tx_validation(void)
{
    validated = 0;

    bool app_loading = app_loading_stop();

    ui_init(ux_sign_tx_flow);

    while (validated == 0) {
        int button = wait_button();
        ui_button_helper(button);
    }

    if (app_loading) {
        app_loading_start();
    } else {
        ux_idle();
    }

    return (validated == 1);
}
