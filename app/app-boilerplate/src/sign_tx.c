#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app-boilerplate.h"
#include "sdk.h"

#include "ux/glyphs.h"
#include "ux/ux.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static char g_tx[16];
static int validated;

UX_STEP_NOCB(ux_sign_tx_1_step, pn, {&C_icon_eye, "Confirm signature?"});
UX_STEP_NOCB(ux_sign_tx_2_step, nn, {"Data:", g_tx});
UX_STEP_CB(ux_sign_tx_3_step, pn, validated = 1, {&C_icon_validate_14, "Approve"});
UX_STEP_CB(ux_sign_tx_4_step, pn, validated = -1, {&C_icon_crossmark, "Reject"});

UX_FLOW(ux_sign_tx_flow,
        &ux_sign_tx_1_step,
        &ux_sign_tx_2_step,
        &ux_sign_tx_3_step,
        &ux_sign_tx_4_step);

static void ui_button_helper(int button)
{
    unsigned int button_mask = button | BUTTON_EVT_RELEASED;

    if (G_ux.stack[0].button_push_callback != NULL) {
        G_ux.stack[0].button_push_callback(button_mask, 0);
    }
}

/* return true if the user approved the tx, false otherwise */
static bool user_validation(void)
{
    validated = 0;

    memset(&G_ux, 0, sizeof(G_ux));
    ux_stack_push();
    ux_flow_init(0, ux_sign_tx_flow, NULL);

    while (validated == 0) {
        int button = wait_button();
        ui_button_helper(button);
    }

    return (validated == 1);
}

static void set_tx_info(MessageSignTx *msg)
{
    size_t size = msg->tx.len;

    if (size > sizeof(g_tx) - 1) {
        memcpy(g_tx, msg->tx.data, sizeof(g_tx) - 4);
        memcpy(g_tx + sizeof(g_tx) - 4, "...", 3);
        size = sizeof(g_tx) - 1;
    } else {
        memcpy(g_tx, msg->tx.data, size);
    }

    g_tx[size] = '\x00';
}

ResponseSignTx *handle_sign_tx(MessageSignTx *msg)
{
    set_tx_info(msg);

    ALLOC_RESPONSE(ResponseSignTx, RESPONSE_SIGN_TX__INIT);
    response->approved = user_validation();

    if (response->approved) {
        response->signature.data = malloc(32);
        response->signature.len = 32;
        /* XXX: not really a signature :> */
        sha256sum(msg->tx.data, msg->tx.len, response->signature.data);
    }

    return response;
}

