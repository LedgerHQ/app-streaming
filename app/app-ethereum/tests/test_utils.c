#include <setjmp.h>
#include <stdio.h>

#include <cmocka.h>

#include "app-ethereum.h"

static void convertUint256BE(const uint8_t *data, const size_t length, uint256_t *target)
{
    uint8_t tmp[INT256_LENGTH];

    memset(tmp, 0, sizeof(tmp) - length);
    memcpy(tmp + sizeof(tmp) - length, data, length);

    readu256BE(tmp, target);
}

static void test_utils(void **state __attribute__((unused)))
{
    uint256_t amount;
    char buf[256];
    uint64_t chain_id = 3;

    convertUint256BE((const uint8_t *)"\x01", 1, &amount);

    compute_amount(chain_id, &amount, buf, sizeof(buf));
    assert_string_equal(buf, "ETH 0.000000000000000001");

    uint256_t gas_limit, gas_price;
    convertUint256BE((const uint8_t *)"\x3b\x9a\xca\x00", 4, &gas_price);
    convertUint256BE((const uint8_t *)"\x01\x5f\x90", 3, &gas_limit);

    compute_fees(chain_id, &gas_price, &gas_limit, buf, sizeof(buf));
    assert_string_equal(buf, "ETH 0.00009");
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_utils),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
