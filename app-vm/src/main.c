#include <stdint.h>
#include <string.h>

#include "os.h"
#include "ux.h"
#include "os_io_seproxyhal.h"


/*#include "types.h"
#include "globals.h"
#include "io.h"
#include "sw.h"
#include "ui/menu.h"
#include "apdu/parser.h"
#include "apdu/dispatcher.h"*/


#define OFFSET_CLA 0
#define OFFSET_INS 1
#define OFFSET_P1 2
#define OFFSET_P2 3
#define OFFSET_LC 4
#define OFFSET_CDATA 5

typedef enum {
    GET_VERSION = 0x03,     /// version of the application
    GET_APP_NAME = 0x04,    /// name of the application
    GET_PUBLIC_KEY = 0x05,  /// public key of corresponding BIP32 path
    SIGN_TX = 0x06          /// sign transaction with BIP32 path
} command_e;

uint8_t G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];
//io_state_e G_io_state;
ux_state_t G_ux;
bolos_ux_params_t G_ux_params;
//global_ctx_t G_context;

typedef struct {
    uint8_t cla;    /// Instruction class
    command_e ins;  /// Instruction code
    uint8_t p1;     /// Instruction parameter 1
    uint8_t p2;     /// Instruction parameter 2
    uint8_t lc;     /// Lenght of command data
    uint8_t *data;  /// Command data
} command_t;

static bool apdu_parser(command_t *cmd, uint8_t *buf, size_t buf_len) {
    // Check minimum length and Lc field of APDU command
    if (buf_len < OFFSET_CDATA || buf_len - OFFSET_CDATA != buf[OFFSET_LC]) {
        return false;
    }

    cmd->cla = buf[OFFSET_CLA];
    cmd->ins = (command_e) buf[OFFSET_INS];
    cmd->p1 = buf[OFFSET_P1];
    cmd->p2 = buf[OFFSET_P2];
    cmd->lc = buf[OFFSET_LC];
    cmd->data = (buf[OFFSET_LC] > 0) ? buf + OFFSET_CDATA : NULL;

    return true;
}

void io_seproxyhal_display(const bagl_element_t *element) {
    io_seproxyhal_display_default((bagl_element_t *) element);
}

uint8_t io_event(uint8_t channel __attribute__((unused))) {
    switch (G_io_seproxyhal_spi_buffer[0]) {
        case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT:
            UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
            break;
        case SEPROXYHAL_TAG_STATUS_EVENT:
            if (G_io_apdu_media == IO_APDU_MEDIA_USB_HID &&  //
                !(U4BE(G_io_seproxyhal_spi_buffer, 3) &      //
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

static void app_main_(void)
{
    command_t cmd;
    int ret;

    memset(&cmd, 0, sizeof(cmd));

    while (1) {
        ret = io_exchange(CHANNEL_APDU, 0);

        if (!apdu_parser(&cmd, G_io_apdu_buffer, ret)) {
            //PRINTF("=> /!\\ BAD LENGTH: %.*H\n", ret, G_io_apdu_buffer);
            //io_send_sw(SW_WRONG_DATA_LENGTH);
            continue;
        }

        stream_init_app(NULL);
        stream_run_app();

        os_sched_exit(-1);
    }
}

void app_main(void) {
    for (;;) {
        BEGIN_TRY {
            TRY {
                app_main_();
            }
            CATCH(EXCEPTION_IO_RESET) {
                THROW(EXCEPTION_IO_RESET);
            }
            CATCH_OTHER(e) {
                G_io_apdu_buffer[0] = e >> 8;
                G_io_apdu_buffer[1] = e & 0xff;
                io_exchange(CHANNEL_APDU | IO_ASYNCH_REPLY, 2);
            }
            FINALLY {
            }
            END_TRY;
        }
    }
}

/**
 * Exit the application and go back to the dashboard.
 */
void app_exit() {
    BEGIN_TRY_L(exit) {
        TRY_L(exit) {
            os_sched_exit(-1);
        }
        FINALLY_L(exit) {
        }
    }
    END_TRY_L(exit);
}

/**
 * Main loop to setup USB, Bluetooth, UI and launch app_main().
 */
__attribute__((section(".boot"))) int main() {
    __asm volatile("cpsie i");

    os_boot();

    for (;;) {
        // Reset UI
        memset(&G_ux, 0, sizeof(G_ux));

        BEGIN_TRY {
            TRY {
                io_seproxyhal_init();

#ifdef TARGET_NANOX
                G_io_app.plane_mode = os_setting_get(OS_SETTING_PLANEMODE, NULL, 0);
#endif  // TARGET_NANOX

                USB_power(0);
                USB_power(1);

                //ui_menu_main();

#ifdef HAVE_BLE
                BLE_power(0, NULL);
                BLE_power(1, "Nano X");
#endif  // HAVE_BLE
                app_main();
            }
            CATCH(EXCEPTION_IO_RESET) {
                CLOSE_TRY;
                continue;
            }
            CATCH_ALL {
                CLOSE_TRY;
                break;
            }
            FINALLY {
            }
        }
        END_TRY;
    }

    app_exit();

    return 0;
}
