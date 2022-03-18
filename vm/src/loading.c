#include <stdlib.h>

#include "ecall.h"
#include "loading.h"
#include "ux.h"

#include "os_io_seproxyhal.h"

#define TICKER_THRESHOLD       175
#define INSTRUCTIONS_THRESHOLD 2000

#define ARRAYLEN(array) (sizeof(array) / sizeof(array[0]))

static size_t app_loading_counter;
static size_t app_loading_ticker;
static bool app_loading;

typedef struct ux_layout_p_params_s {
    const bagl_icon_details_t* icon;
	const char* line1;
} ux_layout_p_params_t;


const bagl_element_t ux_layout_p_elements[] = {
#if (BAGL_WIDTH==128 && BAGL_HEIGHT==64)
  {{BAGL_RECTANGLE                      , 0x00,   0,   0, 128,  64, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0}, NULL},

  {{BAGL_ICON                           , 0x10,  57,  17,  14,  14, 0, 0, 0        , 0xFFFFFF, 0x000000, BAGL_FONT_OPEN_SANS_REGULAR_11px|BAGL_FONT_ALIGNMENT_CENTER, 0  }, NULL },
  {{BAGL_LABELINE                       , 0x11,   0,  44, 128,  32, 0, 0, 0        , 0xFFFFFF, 0x000000, BAGL_FONT_OPEN_SANS_REGULAR_11px|BAGL_FONT_ALIGNMENT_CENTER, 0  }, NULL },
#elif (BAGL_WIDTH==128 && BAGL_HEIGHT==32)
  // erase
  {{BAGL_RECTANGLE                      , 0x00,   0,   0, 128,  32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0}, NULL},

  {{BAGL_ICON                           , 0x10,  56,  2,  16,  16, 0, 0, 0        , 0xFFFFFF, 0x000000, BAGL_FONT_OPEN_SANS_REGULAR_11px|BAGL_FONT_ALIGNMENT_CENTER, 0  }, NULL },
  {{BAGL_LABELINE                       , 0x11,   0, 28, 128,  32, 0, 0, 0        , 0xFFFFFF, 0x000000, BAGL_FONT_OPEN_SANS_REGULAR_11px|BAGL_FONT_ALIGNMENT_CENTER, 0  }, NULL },
#else
  #error "BAGL_WIDTH/BAGL_HEIGHT not defined"
#endif
};

static const bagl_element_t* ux_layout_p_prepro(const bagl_element_t* element)
{
    const ux_layout_p_params_t* params = (const ux_layout_p_params_t*)ux_stack_get_current_step_params();

    memmove(&G_ux.tmp_element, element, sizeof(bagl_element_t));

    switch (element->component.userid) {
    case 0x10:
        G_ux.tmp_element.text = (const char*)params->icon;
        break;
    case 0x11:
  		G_ux.tmp_element.text = params->line1;
    }

    return &G_ux.tmp_element;
}

static void ux_layout_p_init(unsigned int stack_slot)
{
    ux_stack_init(stack_slot);
    G_ux.stack[stack_slot].element_arrays[0].element_array = ux_layout_p_elements;
    G_ux.stack[stack_slot].element_arrays[0].element_array_count = ARRAYLEN(ux_layout_p_elements);
    G_ux.stack[stack_slot].element_arrays_count = 1;
    G_ux.stack[stack_slot].screen_before_element_display_callback = ux_layout_p_prepro;
    G_ux.stack[stack_slot].button_push_callback = ux_flow_button_callback;
    ux_stack_display(stack_slot);
}

static char loading_status[32];

UX_STEP_NOCB(ux_loading_1_step, p, {&C_icon_loading_1, loading_status});
UX_STEP_NOCB(ux_loading_2_step, p, {&C_icon_loading_2, loading_status});
UX_STEP_NOCB(ux_loading_3_step, p, {&C_icon_loading_3, loading_status});
UX_STEP_NOCB(ux_loading_4_step, p, {&C_icon_loading_4, loading_status});
UX_STEP_NOCB(ux_loading_5_step, p, {&C_icon_loading_5, loading_status});
UX_STEP_NOCB(ux_loading_6_step, p, {&C_icon_loading_6, loading_status});
UX_STEP_NOCB(ux_loading_7_step, p, {&C_icon_loading_7, loading_status});
UX_STEP_NOCB(ux_loading_8_step, p, {&C_icon_loading_8, loading_status});

UX_FLOW(ux_loading_flow,
        &ux_loading_1_step,
        &ux_loading_2_step,
        &ux_loading_3_step,
        &ux_loading_4_step,
        &ux_loading_5_step,
        &ux_loading_6_step,
        &ux_loading_7_step,
        &ux_loading_8_step,
        FLOW_LOOP);

void sys_app_loading_start(uint32_t status_addr)
{
    app_loading = true;
    app_loading_counter = 0;

    if (status_addr == 0) {
        loading_status[0] = '\x00';
    } else {
        /* XXX: this might be wrong if the guest buffer is smaller that sizeof(loading_status) and
         * at the end of a memory mapping */
        copy_guest_buffer(status_addr, loading_status, sizeof(loading_status) - 1);
        loading_status[sizeof(loading_status) - 1] = '\x00';
    }

    memset(&G_ux, 0, sizeof(G_ux));
    ux_stack_push();
    ux_flow_init(0, ux_loading_flow, NULL);
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
        ux_flow_next();
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
