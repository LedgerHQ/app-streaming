#include <stdio.h>
#include <stdlib.h>

#include "sdk.h"
#include "ux/glyphs.h"
#include "ux/ux.h"

#include "swap.h"

static char g_ux_send_amount[50];
static char g_ux_get_amount[50];
static char g_ux_fees[50];

static struct ux_item_s ux_opensea[] = {
    { &C_icon_eye, "Review", "transaction", UX_ACTION_NONE },
    { NULL, "Send", g_ux_send_amount, UX_ACTION_NONE },
    { NULL, "Get", g_ux_get_amount, UX_ACTION_NONE },
    { NULL, "Fees", g_ux_fees, UX_ACTION_NONE },
    { &C_icon_validate_14, "Accept", "and send", UX_ACTION_VALIDATE },
    { &C_icon_crossmark, "Reject", NULL, UX_ACTION_REJECT },
};

/* return true if the user approved the tx, false otherwise */
bool ui_sign_tx_validation(const char *send_amount, const char *get_amount, const char *fees)
{
    strncpy(g_ux_send_amount, send_amount, sizeof(g_ux_send_amount));
    strncpy(g_ux_get_amount, get_amount, sizeof(g_ux_get_amount));
    strncpy(g_ux_fees, fees, sizeof(g_ux_fees));

    bool app_loading = app_loading_stop();

    bool validated = ux_validate(ux_opensea, ARRAY_SIZE(ux_opensea));

    if (app_loading) {
        app_loading_start("Signing transaction...");
    } else {
        ux_idle();
    }

    return validated;
}
