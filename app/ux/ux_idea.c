#include <stdint.h>
#include <stdlib.h>

#include "ux_flow_engine.h"

#include "glyphs.h"
#include "ux.h"

UX_STEP_NOCB(ux_menu_ready_step, pn, {&C_boilerplate_logo, "Fuckin' RISC app"});
UX_STEP_VALID(ux_menu_exit_step, pb, exit(0), {&C_icon_dashboard_x, "Quit"});

UX_FLOW(ux_menu_main_flow,
        &ux_menu_ready_step,
        &ux_menu_exit_step,
        FLOW_LOOP);

void ui_menu_main(void)
{
    memset(&G_ux, 0, sizeof(G_ux));

    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }

    ux_flow_init(0, ux_menu_main_flow, NULL);
}
