#include <setjmp.h>
#include <stdio.h>

/* xxx: for dump_req */
#include <fcntl.h>
#include <unistd.h>

#include <cmocka.h>

#include <pb_common.h>
#include <pb_decode.h>
#include <pb_encode.h>

#include "crypto.h"
#include "swap.h"
#include "swap.pb.h"

static void unhex(char *hex, uint8_t *bin, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        unsigned int c;
        sscanf(&hex[i * 2], "%02x", &c);
        bin[i] = c;
    }
}

bool ui_sign_tx_validation(const char *send_amount, const char *get_amount, const char *fees)
{
    /* bypass validation */
    return true;
}

static void set_partner(Partner *partner, cx_ecfp_private_key_t *partner_privkey)
{
    /* set name */
    strncpy(partner->name, "partner", sizeof(partner->name));
    const uint8_t len = strlen(partner->name);

    /* generate random keypair */
    uint8_t data[32];
    get_random_bytes(data, sizeof(data));
    cx_ecfp_public_key_t partner_pubkey;
    assert_true(ecfp_generate_keypair(CX_CURVE_256K1, data, sizeof(data), &partner_pubkey,
                                      partner_privkey));

    /* set public key */
    memcpy(partner->pubkey, partner_pubkey.W, sizeof(partner->pubkey));

    /* sign partner */
    uint8_t hash[32];
    ctx_sha256_t ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, &len, sizeof(len));
    sha256_update(&ctx, (uint8_t *)partner->name, len);
    sha256_update(&ctx, partner->pubkey, sizeof(partner->pubkey));
    sha256_final(&ctx, hash);

    uint8_t privkey_data[] = "\xb1\xed\x47\xef\x58\xf7\x82\xe2\xbc\x4d\x5a\xbe\x70\xef\x66\xd9\x00"
                             "\x9c\x29\x57\x96\x70\x17\x05\x44\x70\xe0\xf3\xe1\x0f\x58\x33";
    cx_ecfp_private_key_t privkey;
    ecfp_init_private_key(CX_CURVE_256K1, privkey_data, sizeof(privkey_data) - 1, &privkey);

    partner->signature.size = ecdsa_sign(&privkey, CX_RND_RFC6979 | CX_LAST, CX_SHA256, hash,
                                         partner->signature.bytes,
                                         sizeof(partner->signature.bytes));
    assert_int_not_equal(partner->signature.size, 0);
}

static size_t build_tx(const char *payin_address,
                       const char *payin_extra_id,
                       const char *refund_address,
                       const char *refund_extra_id,
                       const char *payout_address,
                       const char *payout_extra_id,
                       const char *currency_from,
                       const char *currency_to,
                       const uint8_t *amount_to_provider,
                       const size_t atp_size,
                       const uint8_t *amount_to_wallet,
                       const size_t atw_size,
                       const char *device_transaction_id,
                       uint8_t *buf,
                       const size_t buf_size)
{
    ledger_swap_NewTransactionResponse response = ledger_swap_NewTransactionResponse_init_default;

    strncpy(response.payin_address, payin_address, sizeof(response.payin_address));
    strncpy(response.refund_address, refund_address, sizeof(response.refund_address));
    strncpy(response.payout_address, payout_address, sizeof(response.payout_address));

    strncpy(response.payin_extra_id, payin_extra_id, sizeof(response.payin_extra_id));
    strncpy(response.refund_extra_id, refund_extra_id, sizeof(response.refund_extra_id));
    strncpy(response.payout_extra_id, payout_extra_id, sizeof(response.payout_extra_id));

    strncpy(response.currency_from, currency_from, sizeof(response.currency_from));
    strncpy(response.currency_to, currency_to, sizeof(response.currency_to));

    memcpy(response.amount_to_provider.bytes, amount_to_provider, atp_size);
    response.amount_to_provider.size = atp_size;

    memcpy(response.amount_to_wallet.bytes, amount_to_wallet, atw_size);
    response.amount_to_wallet.size = atw_size;

    strncpy(response.device_transaction_id, device_transaction_id,
            sizeof(response.device_transaction_id));

    assert_true(response.device_transaction_id[10] == 0);

    pb_ostream_t ostream = pb_ostream_from_buffer(buf, buf_size);
    assert_true(pb_encode(&ostream, ledger_swap_NewTransactionResponse_fields, &response));

    return ostream.bytes_written;
}

static void sign_tx(RequestSwap *req, cx_ecfp_private_key_t *partner_privkey)
{
    uint8_t hash[32];
    ctx_sha256_t ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, req->pb_tx.bytes, req->pb_tx.size);
    sha256_final(&ctx, hash);

    req->signature.size = ecdsa_sign(partner_privkey, CX_RND_RFC6979 | CX_LAST, CX_SHA256, hash,
                                     req->signature.bytes, sizeof(req->signature.bytes));
    assert_int_not_equal(req->signature.size, 0);
}

static void dump_req(RequestSwap *req)
{
    Request r = Request_init_zero;
    r.which_message_oneof = Request_swap_tag;
    r.message_oneof.swap = *req;

    uint8_t buf[4096];

    pb_ostream_t ostream = pb_ostream_from_buffer(buf, sizeof(buf));
    assert_true(pb_encode(&ostream, Request_fields, &r));

    uint32_t size = ostream.bytes_written;
    int fd = open("/app/req.bin", O_WRONLY | O_CREAT | O_TRUNC);
    write(fd, (uint8_t *)&size, sizeof(size));
    write(fd, buf, ostream.bytes_written);
    close(fd);
}

static void test_btc_to_eth(void **state __attribute__((unused)))
{
    RequestSwap req = RequestSwap_init_default;
    swap_ctx_t ctx;

    cx_ecfp_private_key_t partner_privkey;

    req.has_partner = true;
    set_partner(&req.partner, &partner_privkey);
    memset(&ctx.device_id_swap, 'Q', sizeof(ctx.device_id_swap) - 1);
    ctx.device_id_swap[sizeof(ctx.device_id_swap) - 1] = '\x00';

    req.payout_path_count = 5;
    req.payout_path[0] = 0x8000002c;
    req.payout_path[1] = 0x8000003c;
    req.payout_path[2] = 0x80000000;
    req.payout_path[3] = 0;
    req.payout_path[4] = 0;

    req.refund_path_count = 5;
    req.refund_path[0] = 0x80000031;
    req.refund_path[1] = 0x80000000;
    req.refund_path[2] = 0x80000000;
    req.refund_path[3] = 0;
    req.refund_path[4] = 0;

    // 0.00001234
    const uint8_t amount_to_provider[2] = "\x04\xd2";
    // 17136
    const uint8_t fees[2] = "\x42\xf0";
    // (10 ** 18) * 0.04321 // 10^18 wei == 1 ETH
    const uint8_t amount_to_wallet[7] = "\x99\x83\x44\x1c\xbe\xa0\x00";

    memcpy(req.fee.bytes, fees, sizeof(fees));
    req.fee.size = sizeof(fees);

    req.pb_tx.size = build_tx("36skqF7TKdwvLYhdRRdq4kA954qZgZicYB", // payin
                              "",
                              "31mceY4tx8cr75vQLLFcK1Gp2VkGdyZfZy", // refund
                              "",
                              "0xDad77910DbDFdE764fC21FCD4E74D71bBACA6D8D", // payout
                              "", "BTC", "ETH", amount_to_provider, sizeof(amount_to_provider),
                              amount_to_wallet, sizeof(amount_to_wallet),
                              (char *)ctx.device_id_swap, req.pb_tx.bytes, sizeof(req.pb_tx.bytes));

    sign_tx(&req, &partner_privkey);

    //dump_req(&req);

    ResponseSwap response;
    const char *error = handle_swap(&req, &response, &ctx);
    if (error) {
        fprintf(stderr, "%s\n", error);
    }
    assert_null(error);
}

static void test_eth_to_btc(void **state __attribute__((unused)))
{
    RequestSwap req = RequestSwap_init_default;
    swap_ctx_t ctx;

    cx_ecfp_private_key_t partner_privkey;

    req.has_partner = true;
    set_partner(&req.partner, &partner_privkey);
    memset(&ctx.device_id_swap, 'Q', sizeof(ctx.device_id_swap) - 1);
    ctx.device_id_swap[sizeof(ctx.device_id_swap) - 1] = '\x00';

    req.payout_path_count = 5;
    req.payout_path[0] = 0x80000054;
    req.payout_path[1] = 0x80000000;
    req.payout_path[2] = 0x80000000;
    req.payout_path[3] = 1;
    req.payout_path[4] = 0;

    req.refund_path_count = 5;
    req.refund_path[0] = 0x8000002c;
    req.refund_path[1] = 0x8000003c;
    req.refund_path[2] = 0x80000000;
    req.refund_path[3] = 0;
    req.refund_path[4] = 0;

    // BTC 1 (100000000 sat)
    const uint8_t amount_to_wallet[4] = "\x05\xf5\xe1\x00";
    // ETH 0.00084 (840000000000000 wei)
    const uint8_t fees[7] = "\x02\xfb\xf9\xbd\x9c\x80\x00";
    // ETH 1.1234 (1123400000000000000 wei)
    const uint8_t amount_to_provider[8] = "\x0f\x97\x1e\x59\x14\xac\x80\x00";

    memcpy(req.fee.bytes, fees, sizeof(fees));
    req.fee.size = sizeof(fees);

    req.pb_tx.size = build_tx("0xd692Cb1346262F584D17B4B470954501f6715a82", // payin
                              "",
                              "0xDad77910DbDFdE764fC21FCD4E74D71bBACA6D8D", // refund
                              "",
                              "bc1qwpgezdcy7g6khsald7cww42lva5g5dmasn6y2z", // payout
                              "", "ETH", "BTC", amount_to_provider, sizeof(amount_to_provider),
                              amount_to_wallet, sizeof(amount_to_wallet),
                              (char *)ctx.device_id_swap, req.pb_tx.bytes, sizeof(req.pb_tx.bytes));

    sign_tx(&req, &partner_privkey);

    dump_req(&req);

    ResponseSwap response;
    const char *error = handle_swap(&req, &response, &ctx);
    if (error) {
        fprintf(stderr, "%s\n", error);
    }
    assert_null(error);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_btc_to_eth),
        cmocka_unit_test(test_eth_to_btc),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
