#include <stdlib.h>

#include "ecall.h"
#include "error.h"
#include "loading.h"
#include "ux.h"

#include "os_io_seproxyhal.h"
#include "os_screen.h"

#define TICKER_THRESHOLD       175
#define INSTRUCTIONS_THRESHOLD 2000
#define NUMBER_OF_ICONS        8

#define ARRAYLEN(array) (sizeof(array) / sizeof(array[0]))

#if (BAGL_WIDTH==128 && BAGL_HEIGHT==64)
#define TEXT_Y 44
#elif (BAGL_WIDTH==128 && BAGL_HEIGHT==32)
#define TEXT_Y 28
#else
#error "BAGL_WIDTH/BAGL_HEIGHT not defined"
#endif

const bagl_component_t icon_component = {
#if (BAGL_WIDTH==128 && BAGL_HEIGHT==64)
    BAGL_ICON, 0x10, 57, 17, 14, 14, 0, 0, 0, 0xFFFFFF, 0x000000, BAGL_FONT_OPEN_SANS_REGULAR_11px|BAGL_FONT_ALIGNMENT_CENTER, 0
#elif (BAGL_WIDTH==128 && BAGL_HEIGHT==32)
    BAGL_ICON, 0x10, 56,  2, 16, 16, 0, 0, 0, 0xFFFFFF, 0x000000, BAGL_FONT_OPEN_SANS_REGULAR_11px|BAGL_FONT_ALIGNMENT_CENTER, 0
#else
#error "BAGL_WIDTH/BAGL_HEIGHT not defined"
#endif
};

static const bagl_component_t status_component = {
    BAGL_LABELINE, 0x11, 0, TEXT_Y, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000, BAGL_FONT_OPEN_SANS_REGULAR_11px|BAGL_FONT_ALIGNMENT_CENTER, 0
};

static size_t app_loading_counter;
static size_t app_loading_ticker;
static bool app_loading;
static size_t app_loading_step;

static void display_next_icon(void)
{
    /* I don't know how to use PIC */
    const bagl_icon_details_t *loading_icon;
    switch (app_loading_step) {
    default:
    case 0: loading_icon = &C_icon_loading_1; break;
    case 1: loading_icon = &C_icon_loading_2; break;
    case 2: loading_icon = &C_icon_loading_3; break;
    case 3: loading_icon = &C_icon_loading_4; break;
    case 4: loading_icon = &C_icon_loading_5; break;
    case 5: loading_icon = &C_icon_loading_6; break;
    case 6: loading_icon = &C_icon_loading_7; break;
    case 7: loading_icon = &C_icon_loading_8; break;
    }

    io_seproxyhal_display_icon((bagl_component_t *)&icon_component,
                               (bagl_icon_details_t *)loading_icon);
    screen_update();

    app_loading_step = (app_loading_step + 1) % NUMBER_OF_ICONS;
}

bool sys_app_loading_start(guest_pointer_t p_status)
{
    app_loading = true;
    app_loading_counter = 0;

    char loading_status[32];
    if (p_status.addr == 0) {
        loading_status[0] = '\x00';
    } else {
        /* XXX: this might be wrong if the guest buffer is smaller that sizeof(loading_status) and
         * at the end of a memory mapping */
        if (!copy_guest_buffer(p_status, loading_status, sizeof(loading_status) - 1)) {
            return false;
        }
        loading_status[sizeof(loading_status) - 1] = '\x00';
    }

    /* erase screen */
    bagl_draw_bg(0);

    /* draw text */
    bagl_draw_with_context(&status_component, loading_status, strlen(loading_status), BAGL_ENCODING_LATIN1);

    display_next_icon();

    return true;
}

bool sys_app_loading_stop(void)
{
    if (app_loading) {
        app_loading = false;
        return true;
    } else {
        return false;
    }
}

void app_loading_update_ui(bool host_exchange)
{
    if (!app_loading) {
        return;
    }

    bool update_ui = false;

    /* UI updates slow down the app. Don't update the UI for each exchange with
     * the host but use the ticker. */
    if (host_exchange) {
        if (G_io_app.ms >= app_loading_ticker + TICKER_THRESHOLD) {
            app_loading_ticker = G_io_app.ms;
            update_ui = true;
        }
    } else {
        if (app_loading_counter % INSTRUCTIONS_THRESHOLD == 0) {
            update_ui = true;
        }
    }

    if (update_ui) {
        display_next_icon();
    }

}

void app_loading_inc_counter(void)
{
    if (!app_loading) {
        return;
    }

    app_loading_counter++;
    app_loading_update_ui(false);
}
