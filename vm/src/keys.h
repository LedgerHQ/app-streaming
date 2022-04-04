#pragma once

#include <stdbool.h>

#include "cx.h"

struct manifest_s;

struct app_keys_s {
    uint8_t hmac_key[32];
};

struct encrypted_keys_s {
    uint8_t bytes[32];
};

void nv_app_state_init(void);
void derive_app_keys(const uint8_t *app_hash, struct app_keys_s *app_keys);
bool get_device_keys(const uint8_t *app_hash,
                     cx_ecfp_public_key_t *pubkey,
                     struct encrypted_keys_s *encrypted_keys);
bool sign_encrypted_keys(const uint8_t *app_hash,
                         struct encrypted_keys_s *encrypted_keys,
                         uint8_t *sig,
                         size_t *sig_size);
bool verify_manifest_signature(const uint8_t *manifest,
                               const size_t manifest_size,
                               const uint8_t *signature,
                               const size_t signature_size);
bool verify_manifest_pubkey_hash(const uint8_t *app_hash, const uint8_t *pubkey_hash);
