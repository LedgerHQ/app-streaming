#include <setjmp.h>
#include <stdio.h>

#include <cmocka.h>

#include "app-ethereum.h"
#include "crypto.h"

#define PREFIX         "\x19\x45thereum Signed Message:\n"
#define MAX_INT32_SIZE (sizeof("4294967295") - 1)

static void unhex(char *hex, uint8_t *bin, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        unsigned int c;
        sscanf(&hex[i * 2], "%02x", &c);
        bin[i] = c;
    }
}

static void hash_message(const char *msg, const size_t size, uint8_t *hash)
{
    char prefix[sizeof(PREFIX) - 1 + MAX_INT32_SIZE];
    size_t len = snprintf(prefix, sizeof(prefix), PREFIX "%zu", size);

    ctx_sha3_t ctx;
    sha3_256_init(&ctx);
    sha3_256_update(&ctx, (uint8_t *)prefix, len);
    sha3_256_update(&ctx, (uint8_t *)msg, size);
    sha3_256_final(&ctx, hash);
}

static void get_pubkey(const uint32_t *path, const size_t path_count, cx_ecfp_public_key_t *pubkey)
{
    RequestGetPubKey req;

    req.confirm = false;
    req.path_count = path_count;
    req.get_chain_code = false;
    for (size_t i = 0; i < path_count; i++) {
        req.path[i] = path[i];
    }

    ResponseGetPubKey response;
    const char *error = handle_get_pubkey(&req, &response);
    assert_null(error);
    assert_true(response.approved);

    pubkey->curve = CX_CURVE_256K1;
    assert_int_equal(sizeof(pubkey->W), sizeof(response.pubkey));
    memcpy(pubkey->W, response.pubkey, sizeof(pubkey->W));
    pubkey->W_len = sizeof(pubkey->W);
}

static void test_sign_msg(void **state __attribute__((unused)))
{
    RequestSignMsg req;
    const char
        msg[] = "Pour acc\xc3\xa9\x64\x65r \xc3\xa0 TN, merci de bien vouloir signer ce message";

    memset(&req, 0, sizeof(req));

    req.path_count = 4;
    req.path[0] = 0x8000002c;
    req.path[1] = 0x8000003c;
    req.path[2] = 0;
    req.path[3] = 0;

    memcpy(req.message.bytes, msg, sizeof(msg));
    req.message.size = sizeof(msg);

    ResponseSignMsg response;
    const char *error = handle_sign_message(&req, &response);
    assert_null(error);

    cx_ecfp_public_key_t pubkey;
    get_pubkey(req.path, req.path_count, &pubkey);

    uint8_t hash[32];
    hash_message(msg, sizeof(msg), hash);

    assert_true(ecdsa_verify(&pubkey, hash, response.signature.bytes, response.signature.size));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sign_msg),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
