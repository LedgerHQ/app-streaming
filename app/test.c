#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

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

static void _exit(int code)
{
    asm(
        "li t0, 0\n"
        "ecall\n"
        );
}

/* XXX */
double __trunctfdf2 (long double a)
{
    return a;
}

void test_sha256(void)
{
    SHA256_CTX ctx;
    uint8_t data[64] = { 'a' };
    uint8_t hash[32];

    sha256_init(&ctx);
    sha256_update(&ctx, data, sizeof(data));
    sha256_final(&ctx, hash);
}

int main(void)
{
    test_sha256();
    printf("BLAH\n");
    //puts("BLAH %s\n");
    _exit(0);
    return 0;
}
