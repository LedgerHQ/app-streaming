#pragma once

#include <stdbool.h>

#include "cx.h"

struct manifest_s;

void nv_app_state_init(void);
bool sign_manifest(const struct manifest_s *manifest, uint8_t *sig, size_t *sig_size);
bool get_device_pubkey(const uint8_t *app_hash, cx_ecfp_public_key_t *pubkey);
bool verify_manifest_device_signature(const struct manifest_s *manifest,
                                      const uint8_t *signature,
                                      const size_t signature_size);
bool verify_manifest_hsm_signature(const struct manifest_s *manifest,
                                   const uint8_t *signature,
                                   const size_t signature_size);
