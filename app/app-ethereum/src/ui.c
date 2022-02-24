#include <stdio.h>
#include <stdlib.h>

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
