#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app-ethereum.h"
#include "crypto.h"
#include "ui.h"
#include "sdk.h"

#define CHAIN_CODE_SIZE 32

static bool derive_pubkey(const uint32_t *path, size_t path_count, cx_ecfp_public_key_t *pubkey, uint8_t *chain_code)
{
    uint8_t privkey_data[32];
    cx_err_t err = derive_node_bip32(CX_CURVE_256K1, path, path_count, privkey_data, chain_code);
    if (err != CX_OK) {
        goto error;
    }

    cx_ecfp_private_key_t privkey;
    ecfp_init_private_key(CX_CURVE_256K1, privkey_data, sizeof(privkey_data), &privkey);
    err = ecfp_generate_pair(CX_CURVE_256K1, pubkey, &privkey);
    if (err != CX_OK) {
        goto error;
    }

 error:
    explicit_bzero(privkey_data, sizeof(privkey_data));
    explicit_bzero(&privkey, sizeof(privkey));

    return (err == CX_OK) ? true : false;
}

ResponseGetPubKey *handle_get_pubkey(RequestGetPubKey *req)
{
    cx_ecfp_public_key_t pubkey;
    uint8_t chain_code[CHAIN_CODE_SIZE];
    if (!derive_pubkey(req->path, req->n_path, &pubkey, chain_code)) {
        return NULL;
    }

    char address[40];
    uint64_t chainId = 30; /* XXX */
    getEthAddressStringFromKey(&pubkey, address, chainId);

    ALLOC_RESPONSE(ResponseGetPubKey, RESPONSE_GET_PUB_KEY__INIT);

    bool approved;
    if (!protobuf_bool_is_true(req, confirm)) {
        approved = true;
    } else {
        ui_set_pubkey_address(address);
        approved = ui_get_pubkey_validation();
    }

    protobuf_set_bool(response, approved, approved);
    if (approved) {
        protobuf_dup_bytes(response, pubkey, &pubkey.W, 65);
        protobuf_dup_bytes(response, address, address, 40);

        if (protobuf_bool_is_true(req, get_chain_code)) {
            protobuf_dup_bytes(response, chain_code, chain_code, CHAIN_CODE_SIZE);
        }
    }

    return response;
}
