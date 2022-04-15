#include <string.h>

#include "crypto.h"
#include "ecall.h"
#include "sdk.h"

void ecfp_init_private_key(const cx_curve_t curve,
                           const uint8_t *raw_key,
                           const size_t key_len,
                           cx_ecfp_private_key_t *key)
{
    key->curve = curve;
    key->d_len = key_len;
    memcpy(key->d, raw_key, key_len);
}

bool ecfp_generate_keypair(const cx_curve_t curve,
                           const uint8_t *rawkey,
                           const size_t rawkey_size,
                           cx_ecfp_public_key_t *pubkey,
                           cx_ecfp_private_key_t *privkey)
{
    ecfp_init_private_key(curve, rawkey, rawkey_size, privkey);

    return ecall_cx_ecfp_generate_pair(curve, pubkey, privkey, true);
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
    ctx_hash_guest_t ctx;

    ctx.sha256.blen = 0;
    memset(ctx.sha256.block, 0, sizeof(ctx.sha256.block));
    memset(ctx.sha256.acc, 0, sizeof(ctx.sha256.acc));

    if (!hash_update(HASH_ID_SHA256, &ctx, buffer, size)) {
        /* this should never happen unless ctx is corrupted */
        fatal("hash_update SHA256");
    }

    if (!hash_final(HASH_ID_SHA256, &ctx, digest)) {
        /* this should never happen unless ctx is corrupted */
        fatal("hash_final SHA256");
    }
}

void sha3_256(const uint8_t *buffer, size_t size, uint8_t *digest)
{
    ctx_sha3_t ctx;

    sha3_256_init(&ctx);
    sha3_256_update(&ctx, buffer, size);
    sha3_256_final(&ctx, digest);
}

void sha3_256_init(ctx_sha3_t *ctx)
{
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
