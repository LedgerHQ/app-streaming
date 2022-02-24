#include "os.h"
#include "ux.h"
#include "glyphs.h"

#include "ui.h"

static char app_name[32 + 4 + 1];

UX_STEP_NOCB(ux_menu_ready_step, pn, {&C_boilerplate_logo, "RISC-V VM"});
UX_STEP_VALID(ux_menu_exit_step, pb, os_sched_exit(-1), {&C_icon_dashboard_x, "Quit"});

UX_FLOW(ux_menu_main_flow,
        &ux_menu_ready_step,
        &ux_menu_exit_step,
        FLOW_LOOP);

UX_STEP_NOCB(ux_app_idle_main_step, pn, {&C_boilerplate_logo, app_name});
UX_STEP_VALID(ux_app_idle_exit_step, pb, os_sched_exit(-1), {&C_icon_dashboard_x, "Quit"});

UX_FLOW(ux_app_idle_flow,
        &ux_app_idle_main_step,
        &ux_app_idle_exit_step,
        FLOW_LOOP);

void ui_menu_main(void)
{
    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }

    ux_flow_init(0, ux_menu_main_flow, NULL);
}

void set_app_name(char *name)
{
    snprintf(app_name, sizeof(app_name), "%s app", name);
}

void ui_app_idle(void)
{
    memset(&G_ux, 0, sizeof(G_ux));
    ux_stack_push();
    ux_flow_init(0, ux_app_idle_flow, NULL);
}
