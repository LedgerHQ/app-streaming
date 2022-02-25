#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app-ethereum.h"
#include "crypto.h"
#include "ui.h"
#include "sdk.h"

#define CHAIN_CODE_SIZE 32

static char *derive_pubkey(const uint32_t *path, size_t path_count, cx_ecfp_public_key_t *pubkey, uint8_t *chain_code)
{
    char *error = NULL;

    uint8_t privkey_data[32];
    if (derive_node_bip32(CX_CURVE_256K1, path, path_count, privkey_data, chain_code) != CX_OK) {
        error = "path derivation failed";
        goto end;
    }

    cx_ecfp_private_key_t privkey;
    ecfp_init_private_key(CX_CURVE_256K1, privkey_data, sizeof(privkey_data), &privkey);
    if (ecfp_generate_pair(CX_CURVE_256K1, pubkey, &privkey) != CX_OK) {
        error = "failed to generate keypair";
        goto end;
    }

 end:
    explicit_bzero(privkey_data, sizeof(privkey_data));
    explicit_bzero(&privkey, sizeof(privkey));

    return error;
}

char *handle_get_pubkey(RequestGetPubKey *req, ResponseGetPubKey *response)
{
    char *error = NULL;

    cx_ecfp_public_key_t pubkey;
    uint8_t chain_code[CHAIN_CODE_SIZE];
    error = derive_pubkey(req->path, req->path_count, &pubkey, chain_code);
    if (error != NULL) {
        goto end;
    }

    char address[40];
    uint64_t chainId = 30; /* XXX */
    getEthAddressStringFromKey(&pubkey, address, chainId);

    if (!req->confirm) {
        response->approved = true;
    } else {
        ui_set_pubkey_address(address);
        response->approved = ui_get_pubkey_validation();
    }

    if (response->approved) {
        memcpy(response->pubkey, &pubkey.W, 65);
        memcpy(response->address, address, 40);

        if (req->get_chain_code) {
            memcpy(response->chain_code, chain_code, CHAIN_CODE_SIZE);
        }
    }

 end:
    return error;
}
