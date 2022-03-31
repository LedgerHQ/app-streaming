#include <stdbool.h>
#include <string.h>

#include "os_io.h"

#include "apdu.h"
#include "keys.h"

#define INS_GET_DEVICE_KEYS 0x10

struct request_get_device_keys_s {
    uint8_t app_hash[32];
};

struct response_get_device_keys_s {
    uint8_t pubkey_bytes[65];
    struct encrypted_keys_s encrypted_keys;
    uint8_t sig[72];
    uint8_t sig_size;
};

static bool handle_get_device_keys(struct request_get_device_keys_s *request, size_t *tx)
{
    cx_ecfp_public_key_t pubkey;
    struct encrypted_keys_s encrypted_keys;
    if (!get_device_keys(request->app_hash, &pubkey, &encrypted_keys)) {
        return false;
    }

    uint8_t sig[72] = { 0 };
    size_t sig_size = sizeof(sig);
    if (!sign_encrypted_keys(request->app_hash, &encrypted_keys, sig, &sig_size)) {
        return false;
    }

    struct response_get_device_keys_s *response = (struct response_get_device_keys_s *)G_io_apdu_buffer;
    memcpy(response->pubkey_bytes, pubkey.W, sizeof(response->pubkey_bytes));
    memcpy(&response->encrypted_keys, &encrypted_keys, sizeof(response->encrypted_keys));
    memcpy(response->sig, sig, sizeof(sig));
    response->sig_size = sig_size;

    *tx = sizeof(*response);

    return true;
}

size_t handle_general_apdu(uint8_t ins, uint8_t *data)
{
    size_t tx = 0;
    bool success;

    switch (ins) {
    case INS_GET_DEVICE_KEYS:
        success = handle_get_device_keys((struct request_get_device_keys_s *)data, &tx);
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
