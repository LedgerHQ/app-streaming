#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "api/ecall-params.h"
#include "api/uint256.h"
#include "ecall.h"
#include "speculos.h"

#define CX_CURVE_256K1 CX_CURVE_SECP256K1

void ecfp_init_private_key(cx_curve_t curve,
                           const uint8_t *raw_key,
                           size_t key_len,
                           cx_ecfp_private_key_t *key);
bool ecfp_generate_keypair(const cx_curve_t curve,
                           const uint8_t *rawkey,
                           const size_t rawkey_size,
                           cx_ecfp_public_key_t *pubkey,
                           cx_ecfp_private_key_t *privkey);
bool ecfp_get_pubkey(const cx_curve_t curve,
                     const cx_ecfp_private_key_t *privkey,
                     cx_ecfp_public_key_t *pubkey);
bool hash_update(const cx_hash_id_t hash_id,
                 ctx_hash_guest_t *ctx,
                 const uint8_t *buffer,
                 const size_t size);

bool hash_final(const cx_hash_id_t hash_id, ctx_hash_guest_t *ctx, uint8_t *digest);
void sha256sum(const uint8_t *buffer, size_t size, uint8_t *digest);
void sha3_256(const uint8_t *buffer, size_t size, uint8_t *digest);
void sha3_256_init(ctx_sha3_t *ctx);
void sha3_256_update(ctx_sha3_t *ctx, const uint8_t *buffer, const size_t size);
void sha3_256_final(ctx_sha3_t *ctx, uint8_t *digest);

static inline bool derive_node_bip32(cx_curve_t curve,
                                     const unsigned int *path,
                                     size_t path_count,
                                     uint8_t *private_key,
                                     uint8_t *chain)
{
    return ecall_derive_node_bip32(curve, path, path_count, private_key, chain);
}

static inline size_t ecdsa_sign(const cx_ecfp_private_key_t *key,
                                const int mode,
                                const cx_md_t hash_id,
                                const uint8_t *hash,
                                uint8_t *sig,
                                size_t sig_len)
{
    return ecall_ecdsa_sign(key, mode, hash_id, hash, sig, sig_len);
}

static inline bool mult(uint8_t *r, const uint8_t *a, const uint8_t *b, size_t len)
{
    return ecall_mult(r, a, b, len);
}

static inline bool multm(uint8_t *r,
                         const uint8_t *a,
                         const uint8_t *b,
                         const uint8_t *m,
                         size_t len)
{
    return ecall_multm(r, a, b, m, len);
}
