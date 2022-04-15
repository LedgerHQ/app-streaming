#include <string.h>

#include "crypto.h"
#include "tx.h"

/**
 * @return 0 on error
 */
static size_t sign(const uint32_t *path,
                   const size_t path_count,
                   const uint8_t *hash,
                   uint8_t *sig,
                   uint8_t *v)
{
    uint8_t privkey_data[32];
    if (!derive_node_bip32(CX_CURVE_256K1, path, path_count, privkey_data, NULL)) {
        return 0;
    }

    cx_ecfp_private_key_t privkey;
    ecfp_init_private_key(CX_CURVE_256K1, privkey_data, sizeof(privkey_data), &privkey);

    int parity;
    size_t size = extended_ecdsa_sign(&privkey, CX_RND_RFC6979 | CX_LAST, CX_SHA256, hash, sig, MAX_SIG_SIZE, &parity);

    explicit_bzero(privkey_data, sizeof(privkey_data));
    explicit_bzero(&privkey, sizeof(privkey));

    *v = parity;

    return size;
}

/**
 * @return 0 of error
 */
size_t sign_tx(const uint32_t *path,
               const size_t path_size,
               const uint8_t *tx,
               const size_t tx_size,
               uint8_t *buffer,
               const size_t max_size)
{
    uint8_t hash[32];
    sha3_256(tx, tx_size, hash);

    uint8_t sig[MAX_SIG_SIZE];
    uint8_t v;
    size_t sig_size = sign(path, path_size, hash, sig, &v);
    if (sig_size == 0) {
        return 0;
    }

    const int chain_id = 1;
    v += chain_id * 2 + 35;

    uint8_t *p = buffer;
    size_t size = max_size;
    if (!encode_bytes(&v, sizeof(v), &p, &size)) {
        return false;
    }

    // the signature is in DER format
    size_t r_size = sig[3];
    uint8_t *r = &sig[4];
    if (!encode_bytes(r, r_size, &p, &size)) {
        return false;
    }

    size_t s_size = sig[4 + r_size + 1];
    uint8_t *s = &sig[4 + r_size + 2];
    if (!encode_bytes(s, s_size, &p, &size)) {
        return false;
    }

    return max_size - size;
}
