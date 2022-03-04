#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "app-ethereum.h"
#include "crypto.h"
#include "rlp.h"
#include "sdk.h"
#include "ui.h"

static const char *sign_helper(const RequestSignTx *req,
                               const uint8_t *hash,
                               ResponseSignTx_signature_t *signature)
{
    return sign(req->path, req->path_count, hash, signature->bytes, sizeof(signature->bytes),
                &signature->size);
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

    app_loading_start("Preparing UI...");

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

    error = sign_helper(req, hash, &response->signature);
    if (error != NULL) {
        goto end;
    }

end:
    free(tx.to);
    free(tx.data);

    return error;
}
