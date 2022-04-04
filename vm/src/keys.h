#pragma once

#include <stdbool.h>

#include "cx.h"

struct manifest_s;

struct hmac_key_s {
    uint8_t bytes[32];
};

void nv_app_state_init(void);
void derive_hmac_key(const uint8_t *app_hash, struct hmac_key_s *key);
bool sign_manifest(const struct manifest_s *manifest, uint8_t *sig, size_t *sig_size);
bool verify_manifest_device_signature(const struct manifest_s *manifest,
                                      const uint8_t *signature,
                                      const size_t signature_size);
bool verify_manifest_hsm_signature(const struct manifest_s *manifest,
                                   const uint8_t *signature,
                                   const size_t signature_size);
