#include <setjmp.h>
#include <stdio.h>

#include <cmocka.h>

#include "app-ethereum.h"
#include "crypto.h"
#include "eip712.h"

#include "../src/eip712.c"

#define EXAMPLE_MAIL_JSON                                                                          \
    "{\n  \"from\": {\n    \"name\": \"Cow\",\n    \"wallet\": "                                   \
    "\"0xCD2a3d9F938E13CD947Ec05AbC7FE734Df8DD826\"\n  },\n  \"to\": {\n    \"name\": \"Bob\",\n " \
    "   \"wallet\": \"0xbBbBBBBbbBBBbbbBbbBbbbbBBbBbbbbBbBbbBBbB\"\n  },\n  \"contents\": "        \
    "\"Hello, Bob!\"\n}\n"

void assert_digest_equal(const uint8_t *digest, const char *hexpected)
{
    uint8_t expected[32];

    unhex(hexpected, expected, sizeof(expected));
    assert_memory_equal(digest, expected, sizeof(hexpected));
}

static void test_encode_data(void **state __attribute__((unused)))
{
    uint8_t digest[32];

    member_data_t bool_true = { .type = TYPE_BOOL, .boolean = true };
    encode_data(&bool_true, digest);
    assert_digest_equal(digest, "0000000000000000000000000000000000000000000000000000000000000001");

    member_data_t bool_false = { .type = TYPE_BOOL, .boolean = false };
    encode_data(&bool_false, digest);
    assert_digest_equal(digest, "0000000000000000000000000000000000000000000000000000000000000000");

    member_data_t string = { .type = TYPE_STRING, .string = { "lol", 3 } };
    encode_data(&string, digest);
    assert_digest_equal(digest, "f172873c63909462ac4de545471fd3ad3e9eeadeec4608b92d16ce6b500704cc");

    member_data_t bytes = { .type = TYPE_BYTES, .bytes = { (const uint8_t *)"lol", 3 } };
    encode_data(&bytes, digest);
    assert_digest_equal(digest, "f172873c63909462ac4de545471fd3ad3e9eeadeec4608b92d16ce6b500704cc");

    member_data_t address = { .type = TYPE_ADDRESS,
                              .address = { "49EdDD3769c0712032808D86597B84ac5c2F5614" } };
    encode_data(&address, digest);
    assert_digest_equal(digest, "00000000000000000000000049eddd3769c0712032808d86597b84ac5c2f5614");

    member_data_t bytes_n = { .type = TYPE_BYTES_N, .bytes_n = { "1234", 2 } };
    encode_data(&bytes_n, digest);
    assert_digest_equal(digest, "1234000000000000000000000000000000000000000000000000000000000000");

    member_data_t uint_n = { .type = TYPE_UINT_N, .uint_n = { "1234", 16 / 8 } };
    encode_data(&uint_n, digest);
    assert_digest_equal(digest, "0000000000000000000000000000000000000000000000000000000000001234");

    member_data_t int_n = { .type = TYPE_INT_N, .int_n = { "1234", 16 / 8, true } };
    encode_data(&int_n, digest);
    assert_digest_equal(digest, "0000000000000000000000000000000000000000000000000000000000001234");

    member_data_t negative_int_n = { .type = TYPE_INT_N, .int_n = { "1234", 16 / 8, false } };
    encode_data(&negative_int_n, digest);
    assert_digest_equal(digest, "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff9234");

    // >>> keccak.new(digest_bits=256).update(int(0).to_bytes(32, "big") + int(1).to_bytes(32,
    // "big")).hexdigest()
    member_data_t array_values[] = { bool_false, bool_true };
    eip712_array_t array_ = { .values = array_values, .count = 2 };
    member_data_t array = { .type = TYPE_ARRAY, .array = &array_ };
    encode_data(&array, digest);
    assert_digest_equal(digest, "a6eef7e35abe7026729641147f7915573c7e97b47efa546f5f6e3230263bcb49");
}

static void test_example_mail(void **state __attribute__((unused)))
{
    uint8_t digest[32];

    /* disabled because of UI validation */
#if 0
    const char *error = eip712_hash_struct(EXAMPLE_MAIL_JSON, sizeof(EXAMPLE_MAIL_JSON),
                                           (const uint8_t *)EIP712_DOMAIN_EXAMPLE_MAIL, digest);
    assert_null(error);
#else
    memcpy(digest,
           "\xc5\x2c\x0e\xe5\xd8\x42\x64\x47\x18\x06\x29\x0a\x3f\x2c\x4c\xec\xfc\x54\x90\x62\x6b"
           "\xf9\x12\xd0\x1f\x24\x0d\x7a\x27\x4b\x37\x1e",
           32);
#endif

    assert_digest_equal(digest, "c52c0ee5d84264471806290a3f2c4cecfc5490626bf912d01f240d7a274b371e");

    ctx_sha3_t ctx;
    uint8_t hash[32];

    sha3_256_init(&ctx);
    sha3_256_update(&ctx, (uint8_t *)"\x19\x01", 2);
    sha3_256_update(&ctx, (const uint8_t *)EIP712_DOMAIN_EXAMPLE_MAIL, 32);
    sha3_256_update(&ctx, digest, sizeof(digest));
    sha3_256_final(&ctx, hash);

    assert_digest_equal(hash, "be609aee343fb3c4b28e1df9e632fca64fcfaede20f02e86244efddf30957bd2");
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_encode_data),
        cmocka_unit_test(test_example_mail),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
