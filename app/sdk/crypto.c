#include <string.h>

#include "crypto.h"
#include "ecall.h"

void ecfp_init_private_key(cx_curve_t curve,
                           const uint8_t *raw_key,
                           size_t key_len,
                           cx_ecfp_private_key_t *key)
{
    key->curve = curve;

    if (raw_key != NULL) {
        key->d_len = key_len;
        memcpy(key->d, raw_key, key_len);
    }
}

void sha3_256_init(ctx_sha3_t *ctx)
{
    const size_t size = 256;
    ctx->blen = 0;
    ctx->output_size = size >> 3;
    ctx->block_size = (1600 - 2 * size) >> 3;
    memset(ctx->block, 0, sizeof(ctx->block));
    memset(ctx->acc, 0, sizeof(ctx->acc));
}

void sha3_256_update(ctx_sha3_t *ctx, const uint8_t *buffer, const size_t size)
{
    if (!hash_update(HASH_ID_SHA3_256, (ctx_hash_guest_t *)ctx, buffer, size)) {
        /* this should never happen unless ctx is corrupted */
        fatal("sha3_256_update");
    }
}

void sha3_256_final(ctx_sha3_t *ctx, uint8_t *digest)
{
    if (!hash_final(HASH_ID_SHA3_256, (ctx_hash_guest_t *)ctx, digest)) {
        /* this should never happen unless ctx is corrupted */
        fatal("sha3_256_final");
    }
}
