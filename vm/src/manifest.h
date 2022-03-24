#pragma once

#include <stdbool.h>

#include "cx.h"

struct manifest_s;

bool get_manifest_pubkey(const char *name, const char *version, cx_ecfp_public_key_t *pubkey);
bool decrypt_manifest(struct manifest_s *manifest, const uint8_t *signature, const size_t signature_size);
