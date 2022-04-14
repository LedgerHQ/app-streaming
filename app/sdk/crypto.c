#include <string.h>

#include "crypto.h"
#include "ecall.h"
#include "sdk.h"

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

/**
 * Get the public key associated to the given private key.
 */
bool ecfp_get_pubkey(const cx_curve_t curve,
                     const cx_ecfp_private_key_t *privkey,
                     cx_ecfp_public_key_t *pubkey)
{
    return ecall_cx_ecfp_generate_pair(curve, pubkey, (cx_ecfp_private_key_t *)privkey, true);
}

bool hash_update(const cx_hash_id_t hash_id,
                 ctx_hash_guest_t *ctx,
                 const uint8_t *buffer,
                 const size_t size)
{
    return ecall_hash_update(hash_id, ctx, buffer, size);
}

bool hash_final(const cx_hash_id_t hash_id, ctx_hash_guest_t *ctx, uint8_t *digest)
{
    return ecall_hash_final(hash_id, ctx, digest);
}

void sha256sum(const uint8_t *buffer, size_t size, uint8_t *digest)
{
    ecall_sha256sum(buffer, size, digest);
}

void sha3_256(const uint8_t *buffer, size_t size, uint8_t *digest)
{
    ecall_sha3_256(buffer, size, digest);
}

void sha3_256_init(ctx_sha3_t *ctx)
{
    const size_t size = 256;
    ctx->blen = 0;
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
