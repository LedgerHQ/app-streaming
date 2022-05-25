#include <pb_common.h>
#include <pb_decode.h>

#include "currency_config.h"
#include "swap.h"
#include "swap.pb.h"

static bool verify_tx_signature(const uint8_t *raw_pubkey,
                                const uint8_t *tx,
                                const size_t tx_size,
                                const uint8_t *signature,
                                size_t signature_size)
{
    uint8_t hash[32];
    sha256(tx, tx_size, hash);

    cx_ecfp_public_key_t pubkey;
    ecfp_init_public_key(CX_CURVE_256K1, raw_pubkey, &pubkey);

    return ecdsa_verify(&pubkey, hash, signature, signature_size);
}

const char *handle_init_swap(const RequestInitSwap *req,
                             ResponseInitSwap *response,
                             swap_ctx_t *ctx)
{
    for (size_t i = 0; i < sizeof(ctx->device_id_swap) - 1; ++i) {
        uint8_t c;
        get_random_bytes(&c, sizeof(c));
#ifdef TESTING
        c = 42;
#endif
        ctx->device_id_swap[i] = (uint8_t)((int)'A' + c % 26);
    }

    ctx->device_id_swap[sizeof(ctx->device_id_swap) - 1] = '\x00';

    memcpy(response->device_id, ctx->device_id_swap, sizeof(response->device_id));

    return NULL;
}

static bool is_id_valid(swap_ctx_t *ctx)
{
    for (size_t i = 0; i < sizeof(ctx->device_id_swap) - 1; i++) {
        if (ctx->device_id_swap[i] == '\x00') {
            return false;
        }
    }

    return true;
}

const char *handle_swap(const RequestSwap *req, ResponseSwap *response, swap_ctx_t *ctx)
{
    if (!verify_partner(&req->partner)) {
        return "invalid partner";
    }

    ledger_swap_NewTransactionResponse tx;
    pb_istream_t stream = pb_istream_from_buffer(req->pb_tx.bytes, req->pb_tx.size);
    if (!pb_decode(&stream, ledger_swap_NewTransactionResponse_fields, &tx)) {
        return "failed to decode swap tx";
    }

    if (!is_id_valid(ctx)) {
        return "device tx id unset";
    }

    if (memcmp(ctx->device_id_swap, tx.device_transaction_id, sizeof(ctx->device_id_swap)) != 0) {
        return "device tx id doesn't match";
    }

    if (!verify_tx_signature(req->partner.pubkey, req->pb_tx.bytes, req->pb_tx.size,
                             req->signature.bytes, req->signature.size)) {
        return "invalid tx signature";
    }

    const swap_config_t *from = get_config(tx.currency_from);
    const swap_config_t *to = get_config(tx.currency_to);

    if (from == NULL) {
        return "invalid currency from";
    }

    if (to == NULL) {
        return "invalid currency to";
    }

    /* validate payout (ETH) address */
    if (!to->validate_address(tx.payout_address, req->payout_path, req->payout_path_count)) {
        return "invalid payout address";
    }

    /* validate refund (BTC) address */
    if (!from->validate_address(tx.refund_address, req->refund_path, req->refund_path_count)) {
        return "invalid refund address";
    }

    /* get printable send amount (BTC) */
    char send_amount[32];
    if (!from->get_printable_amount(tx.amount_to_provider.bytes, tx.amount_to_provider.size,
                                    send_amount, sizeof(send_amount))) {
        return "invalid send amount";
    }

    /* get printable receive amount (ETH) */
    char recv_amount[32];
    if (!to->get_printable_amount(tx.amount_to_wallet.bytes, tx.amount_to_wallet.size, recv_amount,
                                  sizeof(recv_amount))) {
        return "invalid recv amount";
    }

    /* get printable fees (BTC) */
    char fees[32];
    if (!from->get_printable_amount(req->fee.bytes, req->fee.size, fees, sizeof(fees))) {
        return "failed to get printable fees";
    }

    if (!ui_sign_tx_validation(send_amount, recv_amount, fees)) {
        return "not approved";
    }

    /* XXX */
    if (from->create_tx == NULL) {
        return NULL;
    }

    /* generate tx (BTC) */
    size_t tx_size = from->create_tx(req->refund_path, req->refund_path_count, tx.payin_address,
                                     tx.amount_to_provider.bytes, tx.amount_to_provider.size,
                                     req->fee.bytes, req->fee.size, response->tx.bytes,
                                     sizeof(response->tx.bytes));
    if (tx_size == 0) {
        return "failed to create tx";
    }

    response->tx.size = tx_size;

    return NULL;
}
