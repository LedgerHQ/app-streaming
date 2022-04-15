#include <setjmp.h>
#include <stdio.h>
#include <string.h>

#include <cmocka.h>

#include "eth/eth.h"
#include "eth/tx.h"

static void unhex(char *hex, uint8_t *bin, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        unsigned int c;
        sscanf(&hex[i * 2], "%02x", &c);
        bin[i] = c;
    }
}

static void test_eth_tx(void **state __attribute__((unused)))
{
    const uint8_t amount[7] = "\x99\x83\x44\x1c\xbe\xa0\x00";
    const uint8_t fees[2] = "\x42\xf0";
    uint8_t buffer[512];
    size_t size = rlp_encode_tx("0xd692cb1346262f584d17b4b470954501f6715a82", amount,
                                sizeof(amount), fees, sizeof(fees), buffer, sizeof(buffer));
    assert_int_not_equal(size, 0);

    /*uint8_t expected_tx[95];
    unhex("ef850306dc4200841572f78082520894d692cb1346262f584d17b4b470954501f6715a82879983441cbea000"
          "80018080",
          expected_tx, sizeof(expected_tx));
          assert_memory_equal(buffer, expected_tx, size);*/

    const uint32_t path[] = { 0x8000002c, 0x8000003c, 0x80000000, 0, 0 };

    size_t sig_size = sign_tx(path, 5, buffer, size, buffer + size, sizeof(buffer) - size);
    assert_int_not_equal(sig_size, 0);

    size += sig_size;

    size = rlp_buffer_to_list(buffer, size, sizeof(buffer));
    assert_int_not_equal(size, 0);

    printf("tx: ");
    for (size_t i = 0; i < size; i++) {
        printf("%02x", buffer[i]);
    }
    printf("\n");
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_eth_tx),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
