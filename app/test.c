#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sha256.h"


int puts(const char *str)
{
    asm(
        "li t0, 1\n"
        "add a0, %0, 0\n"
        "ecall\n"
        :: "r"(str) : "t0", "a0"
        );

    return 0;
}

static void test_send(uint32_t code)
{
    asm(
        "li t0, 2\n"
        "add a0, %0, 0\n"
        "ecall\n"
        :: "r"(code) : "t0", "a0"
        );
}

/* XXX */
double __trunctfdf2 (long double a)
{
    return a;
}

void test_sha256(void)
{
    // >>> import hashlib; hashlib.sha256(b"a"*34).hexdigest()
    // 'a27c896c4859204843166af66f0e902b9c3b3ed6d2fd13d435abc020065c526f'

    SHA256_CTX ctx;
    uint8_t data[34];
    uint8_t hash[32];

    memset(data, 'a', sizeof(data));

    sha256_init(&ctx);
    sha256_update(&ctx, data, sizeof(data));
    sha256_final(&ctx, hash);

    char hexdigest[64];
    char hex[16] = "0123456789abcdef";
    uint8_t c;
    size_t i;

    if (memcmp(hash, "\xa2\x7c\x89\x6c\x48\x59\x20\x48\x43\x16\x6a\xf6\x6f\x0e\x90\x2b\x9c\x3b\x3e\xd6\xd2\xfd\x13\xd4\x35\xab\xc0\x20\x06\x5c\x52\x6f", sizeof(hash)) != 0) {
        exit(2);
    }

    for (i = 0; i < sizeof(hash); i++) {
        c = hash[i];
        hexdigest[i*2] = hex[(c >> 4) & 0xf];
        hexdigest[i*2+1] = hex[c & 0xf];
    }

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

int main(void)
{
    //test_sha256();
    //printf("BLAH\n");
    //puts("BLAH %s\n");
    //test_send(0x61626364);
    test_malloc();

    return 0;
}
