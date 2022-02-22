#include <stdlib.h>

#include "sdk.h"
#include "ux/glyphs.h"
#include "ux/ux.h"

#include "ui.h"

UX_STEP_VALID(ux_menu_ready_step, pn, exit(0), {&C_boilerplate_logo, "Boilerplate app"});

UX_FLOW(ux_menu_main_flow,
        &ux_menu_ready_step);

static int validated;
char g_ux_sign_tx_2_step_tx[16];

UX_STEP_NOCB(ux_sign_tx_1_step, pn, {&C_icon_eye, "Confirm signature?"});
UX_STEP_NOCB(ux_sign_tx_2_step, nn, {"Data:", g_ux_sign_tx_2_step_tx});
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

static void ui_init(const ux_flow_step_t * const *steps)
{
    memset(&G_ux, 0, sizeof(G_ux));
    ux_stack_push();
    ux_flow_init(0, steps, NULL);
}

/* return true if the user approved the tx, false otherwise */
bool ui_sign_tx_validation(void)
{
    validated = 0;

    ui_init(ux_sign_tx_flow);

    while (validated == 0) {
        int button = wait_button();
        ui_button_helper(button);
    }

    return (validated == 1);
}

void ui_menu_main(void)
{
    ui_init(ux_menu_main_flow);
}
