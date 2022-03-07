#include <string.h>

#include "cx.h"
#include "os.h"

#include "apdu.h"
#include "ecall.h"
#include "error.h"
#include "rv.h"
#include "stream.h"
#include "types.h"
#include "uint256-internal.h"

#include "sdk/api/ecall-nr.h"

static cx_err_t sys_derive_node_bip32(cx_curve_t curve, uint32_t path_addr, size_t path_count, uint32_t private_key_addr, uint32_t chain_addr)
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

    copy_guest_buffer(path_addr, (void *)path, path_count * sizeof(unsigned int));
    copy_guest_buffer(private_key_addr, private_key, sizeof(private_key));

    os_perso_derive_node_bip32(curve, path, path_count, private_key, chain);
    /* XXX: error? */

    if (private_key_addr != 0) {
        copy_host_buffer(private_key_addr, &private_key, sizeof(private_key));
        explicit_bzero(private_key, sizeof(private_key));
    }

    if (chain_addr != 0) {
        copy_host_buffer(chain_addr, &chain, sizeof(chain));
    }

    return CX_OK;
}

static cx_err_t sys_ecfp_generate_pair(cx_curve_t curve, uint32_t pubkey_addr, uint32_t privkey_addr)
{
    cx_ecfp_public_key_t pubkey;
    cx_ecfp_private_key_t privkey;

    bool keep_private = false;
    if (privkey_addr != 0) {
        keep_private = true;
        copy_guest_buffer(privkey_addr, &privkey, sizeof(privkey));
    }

    cx_err_t err = cx_ecfp_generate_pair_no_throw(curve, &pubkey, &privkey, keep_private);
    if (err != CX_OK) {
        goto error;
    }

    copy_host_buffer(pubkey_addr, &pubkey, sizeof(pubkey));

 error:
    explicit_bzero(&privkey, sizeof(privkey));

    return err;
}

static void sys_sha3_256(uint32_t buffer_addr, size_t size, const uint32_t digest_addr)
{
    cx_sha3_t ctx;

    cx_keccak_init(&ctx, 256);
    while (size > 0) {
        size_t n;
        n = PAGE_SIZE - (buffer_addr - PAGE_START(buffer_addr));
        n = MIN(size, n);

        uint8_t *buffer;
        buffer = get_buffer(buffer_addr, n, false);
        cx_hash((cx_hash_t *)&ctx, 0, buffer, n, NULL, 0);

        buffer_addr += n;
        size -= n;
    }

    uint8_t digest[32];
    cx_hash((cx_hash_t *)&ctx, CX_LAST, NULL, 0, digest, sizeof(digest));

    copy_host_buffer(digest_addr, digest, sizeof(digest));
}

static size_t sys_ecdsa_sign(const uint32_t key_addr, const int mode,
                             const cx_md_t hash_id, const uint32_t hash_addr,
                             uint32_t sig_addr, size_t sig_len)
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

    copy_guest_buffer(key_addr, (void *)&key, sizeof(key));
    copy_guest_buffer(hash_addr, hash, hash_len);

    ret = cx_ecdsa_sign(&key, mode, hash_id, hash, hash_len, sig, sizeof(sig), info);
    if (ret == 0 || ret > sig_len) {
        return 0;
    }

    copy_host_buffer(sig_addr, sig, ret);

    return ret;
}

static void sys_mult(uint32_t r_addr, uint32_t a_addr, uint32_t b_addr, size_t len)
{
    uint8_t r[64], a[32], b[32];

    /* XXX: return an error? */
    if (len > sizeof(a)) {
        fatal("invalid size for mult");
    }

    copy_guest_buffer(a_addr, a, len);
    copy_guest_buffer(b_addr, b, len);

    cx_math_mult(r, a, b, len);

    copy_host_buffer(r_addr, r, len * 2);
}

static void sys_multm(uint32_t r_addr, uint32_t a_addr, uint32_t b_addr, uint32_t m_addr, size_t len)
{
    uint8_t r[64], a[32], b[32], m[32];

    /* XXX: return an error? */
    if (len > sizeof(a)) {
        fatal("invalid size for multm");
    }

    copy_guest_buffer(a_addr, a, len);
    copy_guest_buffer(b_addr, b, len);
    copy_guest_buffer(m_addr, m, len);

    cx_math_multm(r, a, b, m, len);

    copy_host_buffer(r_addr, r, len * 2);
}

static bool sys_tostring256(const uint32_t number_addr, const unsigned int base, uint32_t out_addr, size_t len)
{
    uint256_t number;
    char buf[100];

    if (len > sizeof(buf)) {
        len = sizeof(buf);
    }

    copy_guest_buffer(number_addr, &number, sizeof(number));

    if (!tostring256(&number, base, buf, len)) {
        return false;
    }

    copy_host_buffer(out_addr, buf, len);

    return true;
}

/*
 * XXX - Disclaimer: these ECALLs aren't stable, probably buggy.
 *       The API is a WIP.
 */
bool ecall_bolos(struct rv_cpu *cpu, uint32_t nr)
{
    bool stop = false;

    switch (nr) {
    case ECALL_DERIVE_NODE_BIP32:
        cpu->regs[RV_REG_A0] = sys_derive_node_bip32(cpu->regs[RV_REG_A0], cpu->regs[RV_REG_A1], cpu->regs[RV_REG_A2], cpu->regs[RV_REG_A3], cpu->regs[RV_REG_A4]);
        break;
    case ECALL_CX_ECFP_GENERATE_PAIR:
        cpu->regs[RV_REG_A0] = sys_ecfp_generate_pair(cpu->regs[RV_REG_A0], cpu->regs[RV_REG_A1], cpu->regs[RV_REG_A2]);
        break;
    case ECALL_CX_SHA3_256:
        sys_sha3_256(cpu->regs[RV_REG_A0], cpu->regs[RV_REG_A1], cpu->regs[RV_REG_A2]);
        break;
    case ECALL_ECDSA_SIGN:
        cpu->regs[RV_REG_A0] = sys_ecdsa_sign(cpu->regs[RV_REG_A0], cpu->regs[RV_REG_A1], cpu->regs[RV_REG_A2], cpu->regs[RV_REG_A3], cpu->regs[RV_REG_A4], cpu->regs[RV_REG_A5]);
        break;
    case ECALL_MULT:
        sys_mult(cpu->regs[RV_REG_A0], cpu->regs[RV_REG_A1], cpu->regs[RV_REG_A2], cpu->regs[RV_REG_A3]);
        break;
    case ECALL_MULTM:
        sys_multm(cpu->regs[RV_REG_A0], cpu->regs[RV_REG_A1], cpu->regs[RV_REG_A2], cpu->regs[RV_REG_A3], cpu->regs[RV_REG_A4]);
        break;
    case ECALL_TOSTRING256:
        cpu->regs[RV_REG_A0] = sys_tostring256(cpu->regs[RV_REG_A0], cpu->regs[RV_REG_A1], cpu->regs[RV_REG_A2], cpu->regs[RV_REG_A3]);
        break;
    default:
        sys_exit(0xdeaddead);
        stop = true;
        break;
    }

    return stop;
}
