#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sdk.h"

#include "ux/glyphs.h"
#include "ux/ux.h"

UX_STEP_NOCB(ux_menu_ready_step, pn, {&C_boilerplate_logo, "Fuckin' RISC app"});
UX_STEP_NOCB(ux_menu_lol_step, pn, {&C_boilerplate_logo, "lol lol"});
UX_STEP_VALID(ux_menu_exit_step, pb, exit(0), {&C_icon_dashboard_x, "Quit"});

//UX_STEP_NOCB(ux_menu_ready_step, nn, {"1", "Fuckin' RISC app"});
//UX_STEP_NOCB(ux_menu_lol_step, nn, {"2", "lol"});
//UX_STEP_VALID(ux_menu_exit_step, nn, exit(0), {"3", "Quit"});

UX_FLOW(ux_menu_main_flow,
        &ux_menu_ready_step,
        &ux_menu_lol_step,
        &ux_menu_exit_step,
        FLOW_LOOP);

void ui_button_helper(int button)
{
    unsigned int button_mask;
    unsigned int button_same_mask_counter = 0; // ignored

    button_mask = button | BUTTON_EVT_RELEASED;

    if (G_ux.stack[0].button_push_callback != NULL) {
        G_ux.stack[0].button_push_callback(button_mask, button_same_mask_counter);
    }
}

void ui_menu_main(void)
{
    memset(&G_ux, 0, sizeof(G_ux));

    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }

    ux_flow_init(0, ux_menu_main_flow, NULL);
}

void test_malloc(void)
{
    unsigned char *p = malloc(1024);
    if (p == NULL) {
        exit(9);
    }

    for (size_t i = 0; i < 1024; i++) {
        p[i] = i & 0xff;
    }

    unsigned char c = p[1000];
    if (c != (1000 & 0xff)) {
        exit(1);
    }
}

int main(void)
{
    //test_sha256();
    //printf("BLAH\n");
    //puts("BLAH %s\n");
    //xsend((uint8_t *)"hello\n", 5);
    //test_malloc();
    //test_sha256_2();

    ui_menu_main();
    /*for (int i = 0; i < 10; i++) {
        int button = wait_button();
        uint8_t buf[2] = { '0' + i, '\x00' };
        xsend(buf, sizeof(buf));
    }*/

    //for (int i = 0; i < 10; i++) {
    while (1) {
        int button = wait_button();
        //uint8_t buf[2] = { '0' + i++, '\x00' };
        ui_button_helper(button);
        //xsend(buf, sizeof(buf));
    }

    while (1) {
        uint8_t buf[8192];
        xrecv(buf, sizeof(buf));
    }

    return 0;
}
