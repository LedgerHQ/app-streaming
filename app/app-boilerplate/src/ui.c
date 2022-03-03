#include <stdlib.h>

#include "sdk.h"
#include "ux/glyphs.h"
#include "ux/ux.h"

#include "ui.h"

static int validated;
char g_ux_sign_tx_2_step_tx[16];

UX_STEP_NOCB(ux_sign_tx_1_step, pn, { &C_icon_eye, "Confirm signature?" });
UX_STEP_NOCB(ux_sign_tx_2_step, nn, { "Data:", g_ux_sign_tx_2_step_tx });
UX_STEP_CB(ux_sign_tx_3_step, pn, validated = 1, { &C_icon_validate_14, "Approve" });
UX_STEP_CB(ux_sign_tx_4_step, pn, validated = -1, { &C_icon_crossmark, "Reject" });

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

static void ui_init(const ux_flow_step_t *const *steps)
{
    memset(&G_ux, 0, sizeof(G_ux));
    ux_stack_push();
    ux_flow_init(0, steps, NULL);
}

/* return true if the user approved the tx, false otherwise */
bool ui_sign_tx_validation(void)
{
    bool app_loading;

    validated = 0;

    app_loading = app_loading_stop();

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
