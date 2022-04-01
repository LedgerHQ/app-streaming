#include <stdbool.h>
#include <string.h>

#include "os_io.h"

#include "apdu.h"
#include "keys.h"

#define INS_GET_DEVICE_KEYS 0x10
#define INS_SIGN_APP        0x11

size_t handle_general_apdu(uint8_t ins, uint8_t *data)
{
    size_t tx = 0;
    bool success = false;

    switch (ins) {
    case INS_SIGN_APP:
        success = handle_sign_app((struct cmd_response_app_s *)data, &tx);
        break;
    default:
        break;
    }

    if (!success) {
        G_io_apdu_buffer[0] = 0x61;
        G_io_apdu_buffer[1] = 0x61;
    } else {
        G_io_apdu_buffer[tx] = 0x90;
        G_io_apdu_buffer[tx+1] = 0x00;
    }

    return tx + 2;
}
