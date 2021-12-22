#include "os.h"
#include "ux.h"
#include "glyphs.h"

#include "ui.h"

UX_STEP_NOCB(ux_menu_ready_step, pn, {&C_boilerplate_logo, "RISC-V VM"});
UX_STEP_VALID(ux_menu_exit_step, pb, os_sched_exit(-1), {&C_icon_dashboard_x, "Quit"});

UX_FLOW(ux_menu_main_flow,
        &ux_menu_ready_step,
        &ux_menu_exit_step,
        FLOW_LOOP);

void ui_menu_main(void)
{
    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }

    ux_flow_init(0, ux_menu_main_flow, NULL);
}
