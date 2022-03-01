#include <stdio.h>
#include <setjmp.h>

#include <cmocka.h>

#include "app-ethereum.h"
#include "crypto.h"

static void unhex(char *hex, uint8_t *bin, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        unsigned int c;
        sscanf(&hex[i*2], "%02x", &c);
        bin[i] = c;
    }
}

/* https://www.rfctools.com/ethereum-address-test-tool/ */
static void test_pubkey(void **state __attribute__((unused)))
{
    char address[41];
    uint64_t chain_id = 1;
    cx_ecfp_public_key_t publicKey;

    unhex("00691cd378a84c710992412b53df9f5578fbcc6da3af820d2381a4b079f633948bdd789024b52f3eb8fb842cf86704b4b124fbe7cc9ea175506d658938819a674e",
          publicKey.W, sizeof(publicKey.W));
    getEthAddressStringFromKey(&publicKey, address, chain_id);

    assert_string_equal(address, "C9ACd1a02dDd108b877424346dc5CBDfe4714360");
}

/* https://github.com/ethereum/EIPs/blob/master/EIPS/eip-1191.md */
static void test_pubkey_eip_1191(void **state __attribute__((unused)))
{
    char address[41];
    cx_ecfp_public_key_t publicKey;

    unhex("00691cd378a84c710992412b53df9f5578fbcc6da3af820d2381a4b079f633948bdd789024b52f3eb8fb842cf86704b4b124fbe7cc9ea175506d658938819a674e",
          publicKey.W, sizeof(publicKey.W));

    getEthAddressStringFromKey(&publicKey, address, 30);
    assert_string_equal(address, "c9acD1A02DDd108b877424346Dc5cBDFe4714360");

    getEthAddressStringFromKey(&publicKey, address, 31);
    assert_string_equal(address, "C9acD1a02DDd108b877424346dC5cbDfE4714360");
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_pubkey),
        cmocka_unit_test(test_pubkey_eip_1191),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
