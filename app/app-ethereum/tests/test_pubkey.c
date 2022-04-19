#include <setjmp.h>
#include <stdio.h>

#include <cmocka.h>

#include "app-ethereum.h"
#include "crypto.h"

static void unhex(char *hex, uint8_t *bin, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        unsigned int c;
        sscanf(&hex[i * 2], "%02x", &c);
        bin[i] = c;
    }
}

/* https://www.rfctools.com/ethereum-address-test-tool/ */
static void test_pubkey(void **state __attribute__((unused)))
{
    char address[41];
    uint64_t chain_id = 1;
    cx_ecfp_public_key_t publicKey;

    unhex("0400691cd378a84c710992412b53df9f5578fbcc6da3af820d2381a4b079f633948bdd789024b52f3eb8fb84"
          "2cf86704b4b124fbe7cc9ea175506d658938819a674e",
          publicKey.W, sizeof(publicKey.W));
    getEthAddressStringFromKey(&publicKey, address, chain_id);

    assert_string_equal(address, "48Cea1ae35d23fB6DbB682dc94b3Bd79e80c657E");
}

/* https://github.com/ethereum/EIPs/blob/master/EIPS/eip-1191.md */
static void test_pubkey_eip_1191(void **state __attribute__((unused)))
{
    char address[41];
    cx_ecfp_public_key_t publicKey;

    unhex("0400691cd378a84c710992412b53df9f5578fbcc6da3af820d2381a4b079f633948bdd789024b52f3eb8fb84"
          "2cf86704b4b124fbe7cc9ea175506d658938819a674e",
          publicKey.W, sizeof(publicKey.W));

    getEthAddressStringFromKey(&publicKey, address, 30);
    assert_string_equal(address, "48CEa1aE35d23fB6dbb682dc94b3bd79e80c657e");

    getEthAddressStringFromKey(&publicKey, address, 31);
    assert_string_equal(address, "48cea1AE35d23FB6dbb682dc94B3bD79e80C657E");
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_pubkey),
        cmocka_unit_test(test_pubkey_eip_1191),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
