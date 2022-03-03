#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "app-ethereum.h"
#include "crypto.h"
#include "rlp.h"
#include "sdk.h"
#include "ui.h"

static const char *sign(const uint32_t *path,
                        const size_t path_count,
                        const uint8_t *hash,
                        ResponseSignTx_signature_t *signature)
{
    const char *error = NULL;

    uint8_t privkey_data[32];
    if (derive_node_bip32(CX_CURVE_256K1, path, path_count, privkey_data, NULL) != CX_OK) {
        error = "path derivation failed";
        goto end;
    }

    cx_ecfp_private_key_t privkey;
    ecfp_init_private_key(CX_CURVE_256K1, privkey_data, sizeof(privkey_data), &privkey);

    signature->size = ecdsa_sign(&privkey, CX_RND_RFC6979 | CX_LAST, CX_SHA256, hash,
                                 signature->bytes, sizeof(signature->bytes));
    if (signature->size == 0) {
        error = "ecdsa_sign failed";
        goto end;
    }

end:
    explicit_bzero(privkey_data, sizeof(privkey_data));
    explicit_bzero(&privkey, sizeof(privkey));

    return error;
}

const char *handle_sign_tx(const RequestSignTx *req, ResponseSignTx *response)
{
    const char *error = NULL;

    struct tx_s tx;
    tx.to = NULL;
    tx.data = NULL;

    if (rlp_decode_list(req->raw_tx.bytes, req->raw_tx.size, &tx)) {
        error = "rlp decoding failed";
        goto end;
    }

    ui_set_tx_address(tx.chain_id, tx.to);
    ui_set_tx_amount(tx.chain_id, &tx.value);
    ui_set_tx_network_name(tx.chain_id);
    ui_set_tx_fees(tx.chain_id, &tx.gas_price, &tx.gas_limit);
    response->approved = ui_sign_tx_validation();
    if (!response->approved) {
        goto end;
    }

    uint8_t hash[32];
    sha3_256(req->raw_tx.bytes, req->raw_tx.size, hash);

    error = sign(req->path, req->path_count, hash, &response->signature);
    if (error != NULL) {
        goto end;
    }

end:
    free(tx.to);
    free(tx.data);

    return NULL;
}
