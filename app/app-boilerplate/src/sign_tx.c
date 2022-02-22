#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app-boilerplate.h"
#include "ui.h"
#include "sdk.h"

static void set_tx_info(MessageSignTx *msg)
{
    size_t size = msg->tx.len;

    if (size > sizeof(g_ux_sign_tx_2_step_tx) - 1) {
        memcpy(g_ux_sign_tx_2_step_tx, msg->tx.data, sizeof(g_ux_sign_tx_2_step_tx) - 4);
        memcpy(g_ux_sign_tx_2_step_tx + sizeof(g_ux_sign_tx_2_step_tx) - 4, "...", 3);
        size = sizeof(g_ux_sign_tx_2_step_tx) - 1;
    } else {
        memcpy(g_ux_sign_tx_2_step_tx, msg->tx.data, size);
    }

    g_ux_sign_tx_2_step_tx[size] = '\x00';
}

ResponseSignTx *handle_sign_tx(MessageSignTx *msg)
{
    set_tx_info(msg);

    ALLOC_RESPONSE(ResponseSignTx, RESPONSE_SIGN_TX__INIT);
    response->approved = ui_sign_tx_validation();

    if (response->approved) {
        response->signature.data = malloc(32);
        response->signature.len = 32;
        /* XXX: not really a signature :> */
        sha256sum(msg->tx.data, msg->tx.len, response->signature.data);
    }

    return response;
}

