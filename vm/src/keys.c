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
    uint8_t ecdsa_seed[32];
    uint8_t hmac_seed[32];
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

    cx_get_random_bytes(storage.ecdsa_seed, sizeof(storage.ecdsa_seed));
    cx_get_random_bytes(storage.hmac_seed, sizeof(storage.hmac_seed));
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

void derive_hmac_key(const uint8_t *app_hash, struct hmac_key_s *key)
{
    derive_secret(app_hash, N_storage.hmac_seed, key->bytes);
}

static bool get_privkey(const uint8_t *app_hash, cx_ecfp_private_key_t *privkey)
{
    uint8_t privkey_data[32];
    derive_secret(app_hash, N_storage.ecdsa_seed, privkey_data);

    cx_err_t err = cx_ecfp_init_private_key_no_throw(CX_CURVE_256K1, privkey_data,
                                                     sizeof(privkey_data), privkey);
    explicit_bzero(privkey_data, sizeof(privkey_data));

    if (err != CX_OK) {
        return false;
    }

    return true;
}

static void compute_manifest_hash(const struct manifest_s *manifest, uint8_t *digest)
{
    cx_hash_sha256((uint8_t *)manifest, sizeof(*manifest), digest, CX_SHA256_SIZE);
}

bool sign_manifest(const struct manifest_s *manifest, uint8_t *sig, size_t *sig_size)
{
    uint8_t digest[CX_SHA256_SIZE];
    compute_manifest_hash(manifest, digest);

    cx_ecfp_private_key_t privkey;
    if (!get_privkey(manifest->app_hash, &privkey)) {
        return false;
    }

    cx_err_t err = cx_ecdsa_sign_no_throw(&privkey, CX_RND_RFC6979 | CX_LAST, CX_SHA256,
                                          digest, sizeof(digest), sig, sig_size, NULL);
    explicit_bzero(&privkey, sizeof(privkey));
    if (err != CX_OK) {
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

static bool verify_manifest_signature(const struct manifest_s *manifest,
                               const cx_ecfp_public_key_t *pubkey,
                               const uint8_t *signature,
                               const size_t signature_size)
{
    uint8_t digest[CX_SHA256_SIZE];
    compute_manifest_hash(manifest, digest);

    if (!cx_ecdsa_verify_no_throw(pubkey, digest, sizeof(digest), signature, signature_size)) {
        return false;
    }

    return true;
}

bool verify_manifest_device_signature(const struct manifest_s *manifest,
                                      const uint8_t *signature,
                                      const size_t signature_size)
{
    cx_ecfp_public_key_t pubkey;
    if (!get_device_pubkey(manifest->app_hash, &pubkey)) {
        return false;
    }

    return verify_manifest_signature(manifest, &pubkey, signature, signature_size);
}

static bool get_hsm_pubkey(cx_ecfp_public_key_t *pubkey)
{
    if (cx_ecfp_init_public_key_no_throw(CX_CURVE_256K1, hsm_pubkey_bytes, sizeof(hsm_pubkey_bytes),
                                         pubkey) != CX_OK) {
        return false;
    }

    return true;
}

bool verify_manifest_hsm_signature(const struct manifest_s *manifest,
                                   const uint8_t *signature,
                                   const size_t signature_size)
{
    cx_ecfp_public_key_t pubkey;
    if (!get_hsm_pubkey(&pubkey)) {
        return false;
    }

    return verify_manifest_signature(manifest, &pubkey, signature, signature_size);
}
