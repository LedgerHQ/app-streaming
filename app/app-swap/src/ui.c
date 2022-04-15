/*
Send
BTC 0.0001234

Get
ETH 0.04321

Fees
BTC 0.00017136

Accept
and send
 */

#include <stdio.h>
#include <stdlib.h>

#include "sdk.h"
#include "ux/glyphs.h"
#include "ux/ux.h"

#include "swap.h"

static int validated;

static char g_ux_send_amount[50];
static char g_ux_get_amount[50];
static char g_ux_fees[50];

UX_STEP_NOCB(ux_sign_tx_1_step, pnn, { &C_icon_eye, "Review", "transaction" });
UX_STEP_NOCB(ux_sign_tx_2_step, nn, { "Send", g_ux_send_amount });
UX_STEP_NOCB(ux_sign_tx_3_step, nn, { "Get", g_ux_get_amount });
UX_STEP_NOCB(ux_sign_tx_4_step, nn, { "Fees", g_ux_fees });
UX_STEP_CB(ux_sign_tx_5_step, pbb, validated = 1, { &C_icon_validate_14, "Accept", "and send" });
UX_STEP_CB(ux_sign_tx_6_step, pb, validated = -1, { &C_icon_crossmark, "Reject" });

UX_FLOW(ux_sign_tx_flow,
        &ux_sign_tx_1_step,
        &ux_sign_tx_2_step,
        &ux_sign_tx_3_step,
        &ux_sign_tx_4_step,
        &ux_sign_tx_5_step,
        &ux_sign_tx_6_step);

static void ui_button_helper(int button)
{
    unsigned int button_mask = button | BUTTON_EVT_RELEASED;

    if (G_ux.stack[0].button_push_callback != NULL) {
        G_ux.stack[0].button_push_callback(button_mask, 0);
    }
}

static void ui_init(const ux_flow_step_t *const *steps)
{
    memset(&G_ux, 0, sizeof(G_ux));
    ux_stack_push();
    ux_flow_init(0, steps, NULL);
}

/* return true if the user approved the tx, false otherwise */
bool ui_sign_tx_validation(const char *send_amount, const char *get_amount, const char *fees)
{
    validated = 0;

    strncpy(g_ux_send_amount, send_amount, sizeof(g_ux_send_amount));
    strncpy(g_ux_get_amount, get_amount, sizeof(g_ux_get_amount));
    strncpy(g_ux_fees, fees, sizeof(g_ux_fees));

    bool app_loading = app_loading_stop();

    ui_init(ux_sign_tx_flow);

    while (validated == 0) {
        int button = wait_button();
        ui_button_helper(button);
    }

    if (app_loading) {
        app_loading_start("Signing transaction...");
    } else {
        ux_idle();
    }

    return (validated == 1);
}
