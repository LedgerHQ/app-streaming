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

uint8_t io_event(uint8_t channel __attribute__((unused))) {
    switch (G_io_seproxyhal_spi_buffer[0]) {
        case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT:
            if (!vm_running) {
                UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
            } else {
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

/*
gdb$ bt 10
#0  USBD_HID_DataOut_impl (pdev=<optimized out>, epnum=<optimized out>, buffer=0xda7a023a <G_io_seproxyhal_spi_buffer+6> "\001\001\005") at /home/gab/code/ledger/sdk/sdk-balenos-2.0.0/lib_stusb_impl/usbd_impl.c:1002
#1  0x40005b1c in USBD_LL_DataOutStage (pdev=0xda7a6510 <USBD_Device>, epnum=0x2, pdata=0xda7a023a <G_io_seproxyhal_spi_buffer+6> "\001\001\005") at /home/gab/code/ledger/sdk/sdk-balenos-2.0.0/lib_stusb/STM32_USB_Device_Library/Core/Src/usbd_core.c:315
#2  0x40003de8 in io_seproxyhal_handle_usb_ep_xfer_event () at /home/gab/code/ledger/sdk/sdk-balenos-2.0.0/src/os_io_seproxyhal.c:201
#3  0x40003ebe in io_seproxyhal_handle_event () at /home/gab/code/ledger/sdk/sdk-balenos-2.0.0/src/os_io_seproxyhal.c:290
#4  0x400042e6 in io_exchange (channel=<optimized out>, tx_len=<optimized out>) at /home/gab/code/ledger/sdk/sdk-balenos-2.0.0/src/os_io_seproxyhal.c:1396
#5  0x40004ae2 in stream_request_page (page=0xda7a0640 <app+156>, read_only=0x1) at src/stream.c:189
#6  0x4000529c in get_page (addr=0x10600, page_prot=<optimized out>) at src/stream.c:505
#7  0x400050c0 in mem_read (addr=0x10604, size=0x4) at src/stream.c:549
#8  0x400054ba in stream_run_app () at src/stream.c:657
#9  0x40003918 in app_main_ () at src/main.c:72
missed somewhere probably
*/
                return 1;
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
