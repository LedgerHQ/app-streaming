#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "stream.h"

#include "cx.h"
#include "os_seed.h"
#include "ox.h"

#define SECRET_SIZE 32

const uint8_t hsm_pubkey_bytes[] = { 0x04, 0xc5, 0x41, 0x1c, 0xc0, 0x15, 0x4a, 0xba, 0x30, 0x1a,
                                     0x97, 0x0d, 0xa1, 0x16, 0x05, 0xa5, 0xc6, 0x80, 0x6f, 0x5e,
                                     0x3a, 0x62, 0xde, 0x57, 0x32, 0x4c, 0xd1, 0x4e, 0x63, 0x7a,
                                     0x90, 0x18, 0x01, 0x06, 0xcc, 0x66, 0xf8, 0xb6, 0x09, 0xd7,
                                     0xf1, 0xcb, 0x3d, 0x72, 0x90, 0x4c, 0x7f, 0x6d, 0x83, 0x0c,
                                     0x19, 0x76, 0x10, 0x35, 0xd1, 0xcb, 0x23, 0x85, 0x30, 0x5d,
                                     0x0c, 0xfe, 0x9f, 0xe9, 0x0b };

static void get_privkey_data(const char *name, const char *version, uint8_t *privkey_data)
{
    uint32_t path[13];
    size_t path_count;

    memset(path, 0, sizeof(path));
    path[0] = 0x52495343 | 0x80000000; /* b"RISC".hex() */

    if (0) {
        /* it makes the path derivation way more longer */
        strncpy((char *)&path[1], name, 32);
        strncpy((char *)&path[9], version, 16);
        path_count = 13;
    } else {
        path_count = 1;
    }

    /* XXX: handle THROW */
    os_perso_derive_node_bip32(CX_CURVE_256K1, path, path_count, privkey_data, NULL);

    /* XXX: privkey_data should be xored with a random value stored in the flash
     * to prevent the owner of the seed from decrypting manifests */
}

static bool get_privkey(const char *name, const char *version, cx_ecfp_private_key_t *privkey)
{
    uint8_t privkey_data[32];
    get_privkey_data(name, version, privkey_data);

    cx_err_t err = cx_ecfp_init_private_key_no_throw(CX_CURVE_256K1, privkey_data,
                                                     sizeof(privkey_data), privkey);
    explicit_bzero(&privkey_data, sizeof(privkey_data));

    if (err != CX_OK) {
        return false;
    }

    return true;
}

bool get_manifest_pubkey(const char *name, const char *version, cx_ecfp_public_key_t *pubkey)
{
    cx_ecfp_private_key_t privkey;
    if (!get_privkey(name, version, &privkey)) {
        return false;
    }

    cx_err_t err = cx_ecfp_generate_pair_no_throw(CX_CURVE_256K1, pubkey, &privkey, true);
    explicit_bzero(&privkey, sizeof(privkey));

    if (err != CX_OK) {
        return false;
    }

    return true;
}

static bool get_shared_secret(const char *name,
                              const char *version,
                              const cx_ecfp_public_key_t *pubkey,
                              uint8_t *secret)
{
    cx_ecfp_private_key_t privkey;
    cx_err_t ret;

    get_privkey(name, version, &privkey);
    ret = cx_ecdh_no_throw(&privkey, CX_ECDH_X, pubkey->W, pubkey->W_len, secret, SECRET_SIZE);

    explicit_bzero(&privkey, sizeof(privkey));
    if (ret != CX_OK) {
        return false;
    }

    /* apply a secure key derivation function to the raw Diffie–Hellman shared
     * secret to avoid leaking information about the static private key */
    _Static_assert(SECRET_SIZE == CX_SHA256_SIZE, "invalid SECRET_SIZE");
    cx_hash_sha256(secret, SECRET_SIZE, secret, CX_SHA256_SIZE);

    return true;
}

static bool get_hsm_pubkey(cx_ecfp_public_key_t *pubkey)
{
    if (cx_ecfp_init_public_key_no_throw(CX_CURVE_256K1, hsm_pubkey_bytes, sizeof(hsm_pubkey_bytes),
                                         pubkey) != CX_OK) {
        return false;
    }

    return true;
}

bool decrypt_manifest(struct manifest_s *manifest,
                      const uint8_t *signature,
                      const size_t signature_size)
{
    /* 1. verify signature of the whole manifest */
    cx_ecfp_public_key_t hsm_pubkey;
    if (!get_hsm_pubkey(&hsm_pubkey)) {
        return false;
    }

    uint8_t digest[CX_SHA256_SIZE];
    cx_hash_sha256((uint8_t *)manifest, sizeof(*manifest), digest, sizeof(digest));
    if (!cx_ecdsa_verify_no_throw(&hsm_pubkey, digest, sizeof(digest), signature, signature_size)) {
        return false;
    }

    /* 2. decrypt the encrypted section of the manifest */
    uint8_t secret[32];
    if (!get_shared_secret(manifest->c.name, manifest->c.version, &hsm_pubkey, secret)) {
        return false;
    }

    uint8_t *data = (uint8_t *)&manifest->e;
    size_t size = sizeof(manifest->e);

    cx_aes_key_t key;
    uint8_t iv[CX_AES_BLOCK_SIZE];
    int flag = CX_CHAIN_CBC | CX_DECRYPT;

    cx_aes_init_key_no_throw(secret, sizeof(secret), &key);
    explicit_bzero(&secret, sizeof(secret));

    memset(iv, 0, sizeof(iv));
    cx_aes_iv_no_throw(&key, flag, iv, CX_AES_BLOCK_SIZE, data, size, data, &size);
    explicit_bzero(&key, sizeof(key));

    return true;
}
