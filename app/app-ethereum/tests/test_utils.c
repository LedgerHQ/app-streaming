#include <stdio.h>
#include <setjmp.h>

#include <cmocka.h>

#include "app-ethereum.h"

static void test_utils(void **state __attribute__((unused)))
{
    txInt256_t amount;
    char buf[256];
    uint64_t chain_id = 3;

    memcpy(amount.value, "\x01", 1);
    amount.length = 1;

    compute_amount(chain_id, &amount, buf, sizeof(buf));
    assert_string_equal(buf, "ETH 0.000000000000000001");

    txInt256_t gas_limit, gas_price;
    memcpy(gas_price.value, "\x3b\x9a\xca\x00", 4);
    gas_price.length = 4;

    memcpy(gas_limit.value, "\x01\x5f\x90", 3);
    gas_limit.length = 3;

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
