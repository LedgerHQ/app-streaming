/*
 * Compute the sha256sum of an arbitrary amount of data.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sdk.h"
#include "sha256.h"

static void hexdump(const uint8_t *in, char *out, size_t size)
{
    char hex[16] = "0123456789abcdef";

    for (size_t i = 0; i < size; i++) {
        uint8_t c = in[i];
        out[i*2] = hex[(c >> 4) & 0xf];
        out[i*2+1] = hex[c & 0xf];
    }
}

int main(void)
{
    size_t size;
    xrecvall((uint8_t *)&size, sizeof(size));

    uint8_t *p = malloc(size);
    xrecvall(p, size);

    uint8_t digest[32];

    bool use_syscall = false;

    if (use_syscall) {
        sha256sum(p, size, digest);
    } else {
        SHA256_CTX ctx;
        sha256_init(&ctx);
        sha256_update(&ctx, p, size);
        sha256_final(&ctx, digest);
    }

    free(p);

    char hexdigest[64];
    hexdump(digest, hexdigest, sizeof(digest));

    xsend((uint8_t *)hexdigest, sizeof(hexdigest));

    return 0;
}
