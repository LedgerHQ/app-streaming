#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "glyphs.h"
#include "sdk.h"
#include "sha256.h"
#include "ux.h"

UX_STEP_NOCB(ux_menu_ready_step, pn, {&C_boilerplate_logo, "Fuckin' RISC app"});
UX_STEP_NOCB(ux_menu_lol_step, pn, {&C_boilerplate_logo, "lol lol"});
UX_STEP_VALID(ux_menu_exit_step, pb, exit(0), {&C_icon_dashboard_x, "Quit"});

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

void test_sha256(void)
{
    // >>> import hashlib; hashlib.sha256(b"a"*34).hexdigest()
    // 'a27c896c4859204843166af66f0e902b9c3b3ed6d2fd13d435abc020065c526f'

    SHA256_CTX ctx;
    uint8_t data[512];
    uint8_t hash[32];

    memset(data, 'a', sizeof(data));

    sha256_init(&ctx);
    sha256_update(&ctx, data, sizeof(data));
    sha256_final(&ctx, hash);

    char hexdigest[64];
    char hex[16] = "0123456789abcdef";
    uint8_t c;
    size_t i;

    /*if (memcmp(hash, "\xa2\x7c\x89\x6c\x48\x59\x20\x48\x43\x16\x6a\xf6\x6f\x0e\x90\x2b\x9c\x3b\x3e\xd6\xd2\xfd\x13\xd4\x35\xab\xc0\x20\x06\x5c\x52\x6f", sizeof(hash)) != 0) {
        exit(2);
        }*/

    for (i = 0; i < sizeof(hash); i++) {
        c = hash[i];
        hexdigest[i*2] = hex[(c >> 4) & 0xf];
        hexdigest[i*2+1] = hex[c & 0xf];
    }

    xsend((uint8_t *)hexdigest, sizeof(hexdigest));

    if (memcmp(hexdigest, "a27c896c4859204843166af66f0e902b9c3b3ed6d2fd13d435abc020065c526f", sizeof(hexdigest)) != 0) {
        exit(1);
    }
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

void test_sha256_2(void)
{
    uint8_t hash[32];
    uint8_t *p = malloc(2048);
    size_t n = xrecv(p, 1024);

    sha256sum(p, n, hash);

    char hexdigest[64];
    char hex[16] = "0123456789abcdef";

    for (size_t i = 0; i < sizeof(hash); i++) {
        uint8_t c = hash[i];
        hexdigest[i*2] = hex[(c >> 4) & 0xf];
        hexdigest[i*2+1] = hex[c & 0xf];
    }

    xsend((uint8_t *)hexdigest, sizeof(hexdigest));

    free(p);
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
    while (1) {
        int button = wait_button();
        ui_button_helper(button);
    }

    while (1) {
        uint8_t buf[8192];
        size_t n = xrecv(buf, sizeof(buf));
    }

    return 0;
}
