#include "os.h"
#include "ux.h"
#include "os_io_seproxyhal.h"

void io_seproxyhal_display(const bagl_element_t *element) {
    io_seproxyhal_display_default((bagl_element_t *) element);
}

extern bool vm_running;

/*
void io_seproxyhal_button_push(button_push_callback_t button_callback, unsigned int new_button_mask) {
  if (button_callback) {
    unsigned int button_mask;
    unsigned int button_same_mask_counter;
    // enable speeded up long push
    if (new_button_mask == G_ux_os.button_mask) {
      // each 100ms ~
      G_ux_os.button_same_mask_counter++;
    }

    // when new_button_mask is 0 and 

    // append the button mask
    button_mask = G_ux_os.button_mask | new_button_mask;

    // pre reset variable due to os_sched_exit
    button_same_mask_counter = G_ux_os.button_same_mask_counter;

    // reset button mask
    if (new_button_mask == 0) {
      // reset next state when button are released
      G_ux_os.button_mask = 0;
      G_ux_os.button_same_mask_counter=0;

      // notify button released event
      button_mask |= BUTTON_EVT_RELEASED;
    }
    else {
      G_ux_os.button_mask = button_mask;
    }

    // reset counter when button mask changes
    if (new_button_mask != G_ux_os.button_mask) {
      G_ux_os.button_same_mask_counter=0;
    }

    if (button_same_mask_counter >= BUTTON_FAST_THRESHOLD_CS) {
      // fast bit when pressing and timing is right
      if ((button_same_mask_counter%BUTTON_FAST_ACTION_CS) == 0) {
        button_mask |= BUTTON_EVT_FAST;
      }


      // discard the release event after a fastskip has been detected, to avoid strange at release behavior
      // and also to enable user to cancel an operation by starting triggering the fast skip
      button_mask &= ~BUTTON_EVT_RELEASED;
    }

    // indicate if button have been released
    button_callback(button_mask, button_same_mask_counter);

  }
}
*/

extern int saved_apdu_state;
extern int button_pressed;
extern bool wait_for_button;

uint8_t io_event(uint8_t channel __attribute__((unused))) {
    switch (G_io_seproxyhal_spi_buffer[0]) {
        case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT:
            if (!(vm_running && wait_for_button)) {
                UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
            } else {
                /*
                //if (G_ux.stack[0].button_push_callback) {
                //    io_seproxyhal_button_push(G_ux.stack[0].button_push_callback, G_io_seproxyhal_spi_buffer[3]>>1);
                //}
                if (!io_seproxyhal_spi_is_status_sent()) {
                    io_seproxyhal_general_status();
                }
                saved_apdu_state = G_io_app.apdu_state;
                // break from CAPDU reception
                G_io_app.apdu_state = 0xff;
                G_io_app.apdu_length = 0x7fff;
                //io_seproxyhal_general_status();
                */
                button_pressed = 1 + (G_io_seproxyhal_spi_buffer[3] >> 1);

                //return 1;
            }
            break;
        case SEPROXYHAL_TAG_STATUS_EVENT:
            if (G_io_apdu_media == IO_APDU_MEDIA_USB_HID &&
                !(U4BE(G_io_seproxyhal_spi_buffer, 3) &
                  SEPROXYHAL_TAG_STATUS_EVENT_FLAG_USB_POWERED)) {
                THROW(EXCEPTION_IO_RESET);
            }
            /* fallthrough */
        case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
            UX_DISPLAYED_EVENT({});
            break;
        case SEPROXYHAL_TAG_TICKER_EVENT:
            UX_TICKER_EVENT(G_io_seproxyhal_spi_buffer, {});
            break;
        default:
            UX_DEFAULT_EVENT();
            break;
    }

    if (!io_seproxyhal_spi_is_status_sent()) {
        io_seproxyhal_general_status();
    }

    return 1;
}

uint16_t io_exchange_al(uint8_t channel, uint16_t tx_len) {
    switch (channel & ~(IO_FLAGS)) {
        case CHANNEL_KEYBOARD:
            break;
        case CHANNEL_SPI:
            if (tx_len) {
                io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);

                if (channel & IO_RESET_AFTER_REPLIED) {
                    halt();
                }

                return 0;
            } else {
                return io_seproxyhal_spi_recv(G_io_apdu_buffer, sizeof(G_io_apdu_buffer), 0);
            }
        default:
            THROW(INVALID_PARAMETER);
    }

    return 0;
}
