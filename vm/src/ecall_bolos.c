#include <string.h>

#include "ecall.h"
#include "ecall_hash.h"
#include "error.h"
#include "no_throw.h"
#include "page.h"
#include "uint256-internal.h"

#include "cx.h"
#include "os_random.h"
#include "os_seed.h"

#include "sdk/api/ecall-nr.h"

bool sys_derive_node_bip32(eret_t *eret, cx_curve_t curve, guest_pointer_t p_path, size_t path_count, guest_pointer_t p_private_key, guest_pointer_t p_chain)
{
    const unsigned int path[10];
    uint8_t private_key[32];
    uint8_t chain[32];

    if (path_count > 10) {
        eret->success = false;
        return true;
    }

    if (curve != CX_CURVE_256K1 && curve != CX_CURVE_SECP256R1) {
        fatal("derive_node_bip32: invalid curve (TODO)");
    }

    if (!copy_guest_buffer(p_path, (void *)path, path_count * sizeof(unsigned int))) {
        return false;
    }

    if (!copy_guest_buffer(p_private_key, private_key, sizeof(private_key))) {
        return false;
    }

    if (!os_perso_derive_node_bip32_nt(curve, path, path_count, private_key, chain)) {
        eret->success = false;
        return true;
    }

    if (p_private_key.addr != 0) {
        if (!copy_host_buffer(p_private_key, &private_key, sizeof(private_key))) {
            return false;
        }
        explicit_bzero(private_key, sizeof(private_key));
    }

    if (p_chain.addr != 0) {
        if (!copy_host_buffer(p_chain, &chain, sizeof(chain))) {
            return false;
        }
    }

    eret->success = true;

    return true;
}

bool _sys_cx_ecfp_generate_pair(eret_t *eret, cx_curve_t curve, guest_pointer_t p_pubkey, guest_pointer_t p_privkey, bool keep_privkey)
{
    cx_ecfp_public_key_t pubkey;
    cx_ecfp_private_key_t privkey;

    if (keep_privkey) {
        if (!copy_guest_buffer(p_privkey, &privkey, sizeof(privkey))) {
            return false;
        }

        if (privkey.d_len > sizeof(privkey.d)) {
            eret->success = false;
            goto error;
        }
    }

    cx_err_t err = cx_ecfp_generate_pair_no_throw(curve, &pubkey, &privkey, keep_privkey);
    if (err != CX_OK) {
        eret->success = false;
        goto error;
    }

    if (!copy_host_buffer(p_pubkey, &pubkey, sizeof(pubkey))) {
        return false;
    }

    if (!keep_privkey) {
        if (!copy_host_buffer(p_privkey, &privkey, sizeof(privkey))) {
            return false;
        }
    }

    eret->success = true;

 error:
    explicit_bzero(&privkey, sizeof(privkey));

    return true;
}

bool sys_ecdsa_sign(eret_t *eret, const guest_pointer_t p_key, const int mode,
                    const cx_md_t hash_id, const guest_pointer_t p_hash,
                    guest_pointer_t p_sig, size_t sig_len, guest_pointer_t p_parity)
{
    cx_ecfp_private_key_t key;
    uint8_t hash[CX_SHA512_SIZE];
    size_t hash_len;
    uint8_t sig[100];
    uint32_t info;

    eret->size = 0;

    switch (hash_id) {
    case CX_SHA224: hash_len = CX_SHA224_SIZE; break;
    case CX_SHA256: hash_len = CX_SHA256_SIZE; break;
    case CX_SHA384: hash_len = CX_SHA384_SIZE; break;
    case CX_SHA512: hash_len = CX_SHA512_SIZE; break;
    case CX_RIPEMD160: hash_len = CX_RIPEMD160_SIZE; break;
    default: return true;
    }

    if (!copy_guest_buffer(p_key, (void *)&key, sizeof(key))) {
        return false;
    }
    if (!copy_guest_buffer(p_hash, hash, hash_len)) {
        return false;
    }

    size_t sig_size = sizeof(sig);
    cx_err_t error = cx_ecdsa_sign_no_throw(&key, mode, hash_id, hash, hash_len, sig, &sig_size, &info);
    if (error != CX_OK || sig_size > sig_len) {
        return true;
    }

    if (!copy_host_buffer(p_sig, sig, sig_size)) {
        return false;
    }

    if (p_parity.addr != 0) {
        int int_info = info;
        if (!copy_host_buffer(p_parity, &int_info, sizeof(int_info))) {
            return false;
        }
    }

    eret->size = sig_size;

    return true;
}

bool sys_ecdsa_verify(eret_t *eret, const guest_pointer_t p_key,
                      const guest_pointer_t p_hash,
                      const guest_pointer_t p_sig, const size_t sig_len)
{
    cx_ecfp_public_key_t key;
    uint8_t hash[CX_SHA256_SIZE];
    uint8_t sig[100];

    if (!copy_guest_buffer(p_key, (void *)&key, sizeof(key))) {
        return false;
    }

    if (!copy_guest_buffer(p_hash, hash, sizeof(hash))) {
        return false;
    }

    if (sig_len > sizeof(sig)) {
        eret->success = false;
        return true;
    }

    if (!copy_guest_buffer(p_sig, sig, sig_len)) {
        return false;
    }

    eret->success = cx_ecdsa_verify_no_throw(&key, hash, sizeof(hash), sig, sig_len);

    return true;
}

bool sys_get_random_bytes(guest_pointer_t p_buffer, size_t size)
{
    while (size > 0) {
        const size_t n = BUFFER_MIN_SIZE(p_buffer.addr, size);
        uint8_t *buffer = get_buffer(p_buffer.addr, n, true);
        if (buffer == NULL) {
            return false;
        }

        if (cx_get_random_bytes(buffer, n) != CX_OK) {
            /* this should never happen */
            return false;
        }

        p_buffer.addr += n;
        size -= n;
    }

    return true;
}

/**
 * If m is NULL, mult().
 */
bool sys_multm(eret_t *eret, guest_pointer_t p_r, guest_pointer_t p_a, guest_pointer_t p_b, guest_pointer_t p_m, size_t len)
{
    uint8_t r[64], a[32], b[32];

    if (len > sizeof(a)) {
        err("invalid size for multm");
        eret->success = false;
        return true;
    }

    if (!copy_guest_buffer(p_a, a, len)) {
        return false;
    }

    if (!copy_guest_buffer(p_b, b, len)) {
        return false;
    }

    cx_err_t err;
    if (p_m.addr == 0) {
        err = cx_math_mult_no_throw(r, a, b, len);
    } else {
        uint8_t m[32];
        if (!copy_guest_buffer(p_m, m, len)) {
            return false;
        }

        err = cx_math_multm_no_throw(r, a, b, m, len);
    }

    if (err != CX_OK) {
        eret->success = false;
        return true;
    }

    if (!copy_host_buffer(p_r, r, len * 2)) {
        return false;
    }

    eret->success = true;

    return true;
}

bool sys_tostring256(eret_t *eret, const guest_pointer_t p_number, const unsigned int base, guest_pointer_t p_out, size_t len)
{
    uint256_t number;
    char buf[100];

    if (len > sizeof(buf)) {
        len = sizeof(buf);
    }

    if (!copy_guest_buffer(p_number, &number, sizeof(number))) {
        return false;
    }

    if (!tostring256_implem(&number, base, buf, len)) {
        eret->success = false;
        return true;
    }

    if (!copy_host_buffer(p_out, buf, len)) {
        return false;
    }

    eret->success = true;

    return true;
}
