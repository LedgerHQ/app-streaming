#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "crypto.h"
#include "ecall-vm.h"
#include "ecall.h"
#include "sdk/sdk.h"

#include "../../../vm/src/ecall_hash.h"

cx_err_t ecall_ecfp_get_pubkey(cx_curve_t curve,
                               cx_ecfp_public_key_t *pubkey,
                               const cx_ecfp_private_key_t *privkey)
{
    eret_t eret;

    if (!sys_ecfp_get_pubkey(&eret, curve, NP(pubkey), NP(privkey))) {
        errx(1, "sys_ecfp_get_pubkey failed");
    }

    return eret.error;
}

cx_err_t ecall_derive_node_bip32(cx_curve_t curve,
                                 const unsigned int *path,
                                 size_t path_count,
                                 uint8_t *private_key,
                                 uint8_t *chain)
{
    eret_t eret;

    if (!sys_derive_node_bip32(&eret, curve, NP(path), path_count, NP(private_key), NP(chain))) {
        errx(1, "sys_derive_node_bip32 failed");
    }

    return eret.error;
}

size_t ecall_ecdsa_sign(const cx_ecfp_private_key_t *key,
                        const int mode,
                        const cx_md_t hash_id,
                        const uint8_t *hash,
                        uint8_t *sig,
                        size_t sig_len)
{
    eret_t eret;

    if (!sys_ecdsa_sign(&eret, NP(key), mode, hash_id, NP(hash), NP(sig), sig_len)) {
        errx(1, "sys_ecdsa_sign failed");
    }

    return eret.size;
}

cx_err_t ecall_ecfp_generate_pair(cx_curve_t curve,
                                  cx_ecfp_public_key_t *pubkey,
                                  cx_ecfp_private_key_t *privkey)
{
    eret_t eret;

    if (!sys_ecfp_generate_pair(&eret, curve, NP(pubkey), NP(privkey))) {
        errx(1, "sys_ecfp_generate_pair failed");
    }

    return eret.error;
}

bool ecall_hash_update(const cx_hash_id_t hash_id,
                       ctx_hash_guest_t *ctx,
                       const uint8_t *buffer,
                       const size_t size)
{
    eret_t eret;

    if (!sys_hash_update(&eret, hash_id, NP(ctx), NP(buffer), size)) {
        errx(1, "sys_hash_update failed");
    }

    return eret.boolean;
}

bool ecall_hash_final(const cx_hash_id_t hash_id, ctx_hash_guest_t *ctx, uint8_t *digest)
{
    eret_t eret;

    if (!sys_hash_final(&eret, hash_id, NP(ctx), NP(digest))) {
        errx(1, "sys_hash_final failed");
    }

    return eret.boolean;
}

bool ecall_mult(uint8_t *r, const uint8_t *a, const uint8_t *b, size_t len)
{
    eret_t eret;

    if (!sys_mult(&eret, NP(r), NP(a), NP(b), len)) {
        errx(1, "sys_mult failed");
    }

    return eret.boolean;
}

bool ecall_multm(uint8_t *r, const uint8_t *a, const uint8_t *b, const uint8_t *m, size_t len)
{
    eret_t eret;

    if (!sys_multm(&eret, NP(r), NP(a), NP(b), NP(m), len)) {
        errx(1, "sys_multm failed");
    }

    return eret.boolean;
}

bool ecall_tostring256(const uint256_t *number, const unsigned int base, char *out, size_t len)
{
    eret_t eret;

    if (!sys_tostring256(&eret, NP(number), base, NP(out), len)) {
        errx(1, "sys_tostring256 failed");
    }

    return eret.boolean;
}

void ecall_sha256sum(const uint8_t *buffer, size_t size, uint8_t *digest)
{
    if (!sys_sha256sum(NP(buffer), size, NP(digest))) {
        errx(1, "sys_sha256sum failed");
    }
}

void ecall_sha3_256(const uint8_t *buffer, size_t size, uint8_t *digest)
{
    if (!sys_sha3_256(NP(buffer), size, NP(digest))) {
        errx(1, "sys_sha3_256 failed");
    }
}
