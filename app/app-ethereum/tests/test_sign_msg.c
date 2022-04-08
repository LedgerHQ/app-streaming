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

static void test_sign_msg(void **state __attribute__((unused)))
{
    RequestSignMsg req;

    memset(&req, 0, sizeof(req));

    req.path_count = 4;
    req.path[0] = 0x8000002c;
    req.path[1] = 0x8000003c;
    req.path[2] = 0;
    req.path[3] = 0;

    memcpy(req.message.bytes,
           "Pour acc\xc3\xa9\x64\x65r \xc3\xa0 TN, merci de bien vouloir signer ce message", 60);

    ResponseSignMsg response;
    const char *error = handle_sign_message(&req, &response);
    assert_null(error);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sign_msg),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
