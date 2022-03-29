#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "keys.h"
#include "stream.h"

#include "cx.h"
#include "os_nvm.h"
#include "os_pic.h"
#include "os_random.h"
#include "ox.h"

typedef struct internalStorage_t {
    uint8_t ecdh_seed[32];
    uint8_t app_hmac_seed[32];
    uint8_t app_encryption_seed[32];
    uint8_t initialized;
} internalStorage_t;

static const internalStorage_t N_storage_real;

#define N_storage (*(const internalStorage_t *)PIC(&N_storage_real))

static const uint8_t hsm_pubkey_bytes[] = {
    0x04, 0xc5, 0x41, 0x1c, 0xc0, 0x15, 0x4a, 0xba, 0x30, 0x1a, 0x97, 0x0d, 0xa1,
    0x16, 0x05, 0xa5, 0xc6, 0x80, 0x6f, 0x5e, 0x3a, 0x62, 0xde, 0x57, 0x32, 0x4c,
    0xd1, 0x4e, 0x63, 0x7a, 0x90, 0x18, 0x01, 0x06, 0xcc, 0x66, 0xf8, 0xb6, 0x09,
    0xd7, 0xf1, 0xcb, 0x3d, 0x72, 0x90, 0x4c, 0x7f, 0x6d, 0x83, 0x0c, 0x19, 0x76,
    0x10, 0x35, 0xd1, 0xcb, 0x23, 0x85, 0x30, 0x5d, 0x0c, 0xfe, 0x9f, 0xe9, 0x0b
};

void nv_app_state_init(void)
{
    if (N_storage.initialized == 0x01) {
        return;
    }

    internalStorage_t storage;

    cx_get_random_bytes(storage.ecdh_seed, sizeof(storage.ecdh_seed));
    cx_get_random_bytes(storage.app_hmac_seed, sizeof(storage.app_hmac_seed));
    cx_get_random_bytes(storage.app_encryption_seed, sizeof(storage.app_encryption_seed));
    storage.initialized = 0x01;

    nvm_write((internalStorage_t *)&N_storage, (void *)&storage, sizeof(internalStorage_t));
}

static void derive_secret(const uint8_t *app_hash, const uint8_t *seed, uint8_t *output)
{
    uint8_t secret[64];

    memcpy(secret, seed, 32);
    memcpy(secret + 32, app_hash, 32);

    cx_hash_sha256(secret, sizeof(secret), output, CX_SHA256_SIZE);

    explicit_bzero(secret, sizeof(secret));
}

void derive_app_keys(const uint8_t *app_hash, struct app_keys_s *app_keys)
{
    derive_secret(app_hash, N_storage.app_hmac_seed, app_keys->hmac_key);
    derive_secret(app_hash, N_storage.app_encryption_seed, app_keys->encryption_key);
}

static bool get_privkey(const uint8_t *app_hash, cx_ecfp_private_key_t *privkey)
{
    uint8_t privkey_data[32];
    derive_secret(app_hash, N_storage.ecdh_seed, privkey_data);

    cx_err_t err = cx_ecfp_init_private_key_no_throw(CX_CURVE_256K1, privkey_data,
                                                     sizeof(privkey_data), privkey);
    explicit_bzero(privkey_data, sizeof(privkey_data));

    if (err != CX_OK) {
        return false;
    }

    return true;
}

static bool get_shared_secret(const uint8_t *app_hash,
                              const cx_ecfp_public_key_t *pubkey,
                              uint8_t *secret_key)
{
    cx_ecfp_private_key_t privkey;
    if (!get_privkey(app_hash, &privkey)) {
        return false;
    }

    uint8_t secret[32];
    cx_err_t ret = cx_ecdh_no_throw(&privkey, CX_ECDH_X, pubkey->W, pubkey->W_len, secret, 32);
    explicit_bzero(&privkey, sizeof(privkey));

    if (ret != CX_OK) {
        return false;
    }

    /* kdf */
    cx_hash_sha256(secret, sizeof(secret), secret_key, CX_SHA256_SIZE);
    explicit_bzero(secret, sizeof(secret));

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

static bool get_device_pubkey(const uint8_t *app_hash, cx_ecfp_public_key_t *pubkey)
{
    cx_ecfp_private_key_t privkey;
    if (!get_privkey(app_hash, &privkey)) {
        return false;
    }

    cx_err_t err = cx_ecfp_generate_pair_no_throw(CX_CURVE_256K1, pubkey, &privkey, true);
    explicit_bzero(&privkey, sizeof(privkey));

    if (err != CX_OK) {
        return false;
    }

    return true;
}

bool get_device_keys(const uint8_t *app_hash,
                     cx_ecfp_public_key_t *pubkey,
                     struct encrypted_keys_s *encrypted_keys)
{
    if (!get_device_pubkey(app_hash, pubkey)) {
        return false;
    }

    cx_ecfp_public_key_t hsm_pubkey;
    if (!get_hsm_pubkey(&hsm_pubkey)) {
        return false;
    }

    uint8_t secret_key[32];
    if (!get_shared_secret(app_hash, &hsm_pubkey, secret_key)) {
        return false;
    }

    cx_aes_key_t key;
    uint8_t iv[CX_AES_BLOCK_SIZE];
    int flag = CX_CHAIN_CBC | CX_ENCRYPT;

    cx_aes_init_key_no_throw(secret_key, sizeof(secret_key), &key);
    explicit_bzero(&secret_key, sizeof(secret_key));

    struct app_keys_s app_keys;
    derive_app_keys(app_hash, &app_keys);

    _Static_assert(sizeof(struct app_keys_s) == sizeof(struct encrypted_keys_s),
                   "invalid keys structure");

    memset(iv, 0, sizeof(iv));
    size_t size = sizeof(*encrypted_keys);
    cx_aes_iv_no_throw(&key, flag, iv, CX_AES_BLOCK_SIZE, (const uint8_t *)&app_keys,
                       sizeof(app_keys), encrypted_keys->bytes, &size);
    explicit_bzero(&key, sizeof(key));
    explicit_bzero(&app_keys, sizeof(app_keys));

    return true;
}

bool verify_manifest_signature(const uint8_t *manifest,
                               const size_t manifest_size,
                               const uint8_t *signature,
                               const size_t signature_size)
{
    cx_ecfp_public_key_t hsm_pubkey;
    if (!get_hsm_pubkey(&hsm_pubkey)) {
        return false;
    }

    uint8_t digest[CX_SHA256_SIZE];
    cx_hash_sha256(manifest, manifest_size, digest, sizeof(digest));
    if (!cx_ecdsa_verify_no_throw(&hsm_pubkey, digest, sizeof(digest), signature, signature_size)) {
        return false;
    }

    return true;
}

bool verify_manifest_pubkey_hash(const uint8_t *app_hash, const uint8_t *pubkey_hash)
{
    cx_ecfp_public_key_t pubkey;
    if (!get_device_pubkey(app_hash, &pubkey)) {
        return false;
    }

    uint8_t digest[CX_SHA256_SIZE];
    cx_hash_sha256(pubkey.W, pubkey.W_len, digest, sizeof(digest));

    if (memcmp(digest, pubkey_hash, sizeof(digest)) != 0) {
        return false;
    }

    return true;
}
