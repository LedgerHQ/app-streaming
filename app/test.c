#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sdk.h"
#include "sha256.h"


void test_sha256(void)
{
    // >>> import hashlib; hashlib.sha256(b"a"*34).hexdigest()
    // 'a27c896c4859204843166af66f0e902b9c3b3ed6d2fd13d435abc020065c526f'

    SHA256_CTX ctx;
    uint8_t data[512];
    uint8_t hash[32];

    memset(data, 'a', sizeof(data));

    sha256_init(&ctx);
    sha256_update(&ctx, data, sizeof(data));
    sha256_final(&ctx, hash);

    char hexdigest[64];
    char hex[16] = "0123456789abcdef";
    uint8_t c;
    size_t i;

    /*if (memcmp(hash, "\xa2\x7c\x89\x6c\x48\x59\x20\x48\x43\x16\x6a\xf6\x6f\x0e\x90\x2b\x9c\x3b\x3e\xd6\xd2\xfd\x13\xd4\x35\xab\xc0\x20\x06\x5c\x52\x6f", sizeof(hash)) != 0) {
        exit(2);
        }*/

    for (i = 0; i < sizeof(hash); i++) {
        c = hash[i];
        hexdigest[i*2] = hex[(c >> 4) & 0xf];
        hexdigest[i*2+1] = hex[c & 0xf];
    }

    xsend((uint8_t *)hexdigest, sizeof(hexdigest));

    if (memcmp(hexdigest, "a27c896c4859204843166af66f0e902b9c3b3ed6d2fd13d435abc020065c526f", sizeof(hexdigest)) != 0) {
        exit(1);
    }
}

void test_malloc(void)
{
    unsigned char *p = malloc(1024);
    if (p == NULL) {
        exit(9);
    }

    for (size_t i = 0; i < 1024; i++) {
        p[i] = i & 0xff;
    }

    unsigned char c = p[1000];
    if (c != (1000 & 0xff)) {
        exit(1);
    }
}

void test_sha256_2(void)
{
    uint8_t hash[32];
    uint8_t *p = malloc(2048);
    size_t n = xrecv(p, 1024);

    sha256sum(p, n, hash);

    char hexdigest[64];
    char hex[16] = "0123456789abcdef";

    for (size_t i = 0; i < sizeof(hash); i++) {
        uint8_t c = hash[i];
        hexdigest[i*2] = hex[(c >> 4) & 0xf];
        hexdigest[i*2+1] = hex[c & 0xf];
    }

    xsend((uint8_t *)hexdigest, sizeof(hexdigest));

    free(p);
}

static void test_ux(void)
{
    /*ux_rectangle(0x00000000, 128, 0, 0, 64);
    //ux_rectangle(0xffffffff, 0, 35, 128, 32);

    size_t last_x, last_y;
    last_x = 0;
    last_y = 0;

    for (size_t y = 0; y < 5; y++) {
        for (size_t x = 0; x < 128; x++) {
            ux_rectangle(0x00000000, last_x, last_y, 1, 1);
            ux_rectangle(0xffffffff, x, y, 1, 1);
            last_x = x;
            last_y = y;
            //xsend("a", 1);
        }
        }*/
    ui_menu_main();
}

int main(void)
{
    //test_sha256();
    //printf("BLAH\n");
    //puts("BLAH %s\n");
    //xsend((uint8_t *)"hello\n", 5);
    //test_malloc();
    //test_sha256_2();

    test_ux();

    return 0;
}
