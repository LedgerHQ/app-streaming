#include <string.h>

#include "ecall.h"
#include "ecall_hash.h"
#include "error.h"
#include "rv.h"
#include "types.h"
#include "uint256-internal.h"

#include "cx.h"
#include "os.h"

#include "sdk/api/ecall-nr.h"

cx_err_t sys_derive_node_bip32(cx_curve_t curve, guest_pointer_t p_path, size_t path_count, guest_pointer_t p_private_key, guest_pointer_t p_chain)
{
    const unsigned int path[10];
    uint8_t private_key[32];
    uint8_t chain[32];

    if (path_count > 10) {
        return CX_INVALID_PARAMETER;
    }

    if (curve != CX_CURVE_256K1 && curve != CX_CURVE_SECP256R1) {
        fatal("derive_node_bip32: invalid curve (TODO)");
    }

    copy_guest_buffer(p_path, (void *)path, path_count * sizeof(unsigned int));
    copy_guest_buffer(p_private_key, private_key, sizeof(private_key));

    os_perso_derive_node_bip32(curve, path, path_count, private_key, chain);
    /* XXX: error? */

    if (p_private_key.addr != 0) {
        copy_host_buffer(p_private_key, &private_key, sizeof(private_key));
        explicit_bzero(private_key, sizeof(private_key));
    }

    if (p_chain.addr != 0) {
        copy_host_buffer(p_chain, &chain, sizeof(chain));
    }

    return CX_OK;
}

cx_err_t sys_ecfp_generate_pair(cx_curve_t curve, guest_pointer_t p_pubkey, guest_pointer_t p_privkey)
{
    cx_ecfp_public_key_t pubkey;
    cx_ecfp_private_key_t privkey;

    cx_err_t err = cx_ecfp_generate_pair_no_throw(curve, &pubkey, &privkey, false);
    if (err != CX_OK) {
        goto error;
    }

    copy_host_buffer(p_pubkey, &pubkey, sizeof(pubkey));
    copy_host_buffer(p_privkey, &privkey, sizeof(privkey));

 error:
    explicit_bzero(&privkey, sizeof(privkey));

    return err;
}

cx_err_t sys_ecfp_get_pubkey(cx_curve_t curve, guest_pointer_t p_pubkey, guest_pointer_t p_privkey)
{
    cx_ecfp_public_key_t pubkey;
    cx_ecfp_private_key_t privkey;

    copy_guest_buffer(p_privkey, &privkey, sizeof(privkey));

    cx_err_t err = cx_ecfp_generate_pair_no_throw(curve, &pubkey, &privkey, true);
    if (err != CX_OK) {
        goto error;
    }

    copy_host_buffer(p_pubkey, &pubkey, sizeof(pubkey));

 error:
    explicit_bzero(&privkey, sizeof(privkey));

    return err;
}

size_t sys_ecdsa_sign(const guest_pointer_t p_key, const int mode,
                      const cx_md_t hash_id, const guest_pointer_t p_hash,
                      guest_pointer_t p_sig, size_t sig_len)
{
    cx_ecfp_private_key_t key;
    uint8_t hash[CX_SHA512_SIZE];
    size_t hash_len;
    uint8_t sig[100];
    size_t ret;
    unsigned int *info = NULL;

    switch (hash_id) {
    case CX_SHA224: hash_len = CX_SHA224_SIZE; break;
    case CX_SHA256: hash_len = CX_SHA256_SIZE; break;
    case CX_SHA384: hash_len = CX_SHA384_SIZE; break;
    case CX_SHA512: hash_len = CX_SHA512_SIZE; break;
    case CX_RIPEMD160: hash_len = CX_RIPEMD160_SIZE; break;
    default: return 0;
    }

    copy_guest_buffer(p_key, (void *)&key, sizeof(key));
    copy_guest_buffer(p_hash, hash, hash_len);

    ret = cx_ecdsa_sign(&key, mode, hash_id, hash, hash_len, sig, sizeof(sig), info);
    if (ret == 0 || ret > sig_len) {
        return 0;
    }

    copy_host_buffer(p_sig, sig, ret);

    return ret;
}

void sys_mult(guest_pointer_t p_r, guest_pointer_t p_a, guest_pointer_t p_b, size_t len)
{
    uint8_t r[64], a[32], b[32];

    /* XXX: return an error? */
    if (len > sizeof(a)) {
        fatal("invalid size for mult");
    }

    copy_guest_buffer(p_a, a, len);
    copy_guest_buffer(p_b, b, len);

    cx_math_mult(r, a, b, len);

    copy_host_buffer(p_r, r, len * 2);
}

void sys_multm(guest_pointer_t p_r, guest_pointer_t p_a, guest_pointer_t p_b, guest_pointer_t p_m, size_t len)
{
    uint8_t r[64], a[32], b[32], m[32];

    /* XXX: return an error? */
    if (len > sizeof(a)) {
        fatal("invalid size for multm");
    }

    copy_guest_buffer(p_a, a, len);
    copy_guest_buffer(p_b, b, len);
    copy_guest_buffer(p_m, m, len);

    cx_math_multm(r, a, b, m, len);

    copy_host_buffer(p_r, r, len * 2);
}

bool sys_tostring256(const guest_pointer_t p_number, const unsigned int base, guest_pointer_t p_out, size_t len)
{
    uint256_t number;
    char buf[100];

    if (len > sizeof(buf)) {
        len = sizeof(buf);
    }

    copy_guest_buffer(p_number, &number, sizeof(number));

    if (!tostring256(&number, base, buf, len)) {
        return false;
    }

    copy_host_buffer(p_out, buf, len);

    return true;
}
