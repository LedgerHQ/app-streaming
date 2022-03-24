#include <stdbool.h>
#include <string.h>

#include "os_io.h"

#include "apdu.h"
#include "manifest.h"

#define INS_GET_PUBKEY 0x10

struct request_get_pubkey_s {
    char name[32];
    char version[16];
};

struct response_get_pubkey_s {
    uint8_t pubkey_bytes[65];
};

static bool handle_get_pubkey(struct request_get_pubkey_s *request, size_t *tx)
{
    cx_ecfp_public_key_t pubkey;
    if (!get_manifest_pubkey(request->name, request->version, &pubkey)) {
        return false;
    }

    struct response_get_pubkey_s *response = (struct response_get_pubkey_s *)G_io_apdu_buffer;
    memcpy(response->pubkey_bytes, pubkey.W, sizeof(response->pubkey_bytes));

    *tx = sizeof(response->pubkey_bytes);

    return true;
}

size_t handle_general_apdu(uint8_t ins, uint8_t *data)
{
    size_t tx = 0;
    bool success;

    switch (ins) {
    case INS_GET_PUBKEY:
        success = handle_get_pubkey((struct request_get_pubkey_s *)data, &tx);
        break;
    default:
        success = false;
        break;
    }

    if (!success) {
        G_io_apdu_buffer[0] = 0x61;
        G_io_apdu_buffer[1] = 0x02;
    } else {
        G_io_apdu_buffer[tx] = 0x90;
        G_io_apdu_buffer[tx+1] = 0x00;
    }

    return tx + 2;
}
