#pragma once

/*
 * Don't add speculos as a requirement only for a few defines... (which might
 * be temporary hopefully, since a nice RISC-V SDK may be developed.)
 */

typedef enum cx_curve_e {
    CX_CURVE_SECP256K1 = 0x21,
} cx_curve_t;

typedef struct cx_ecfp_256_public_key_s {
  cx_curve_t curve;
  size_t W_len;
  uint8_t W[65];
} cx_ecfp_public_key_t;

typedef struct cx_ecfp_256_private_key_s {
  cx_curve_t curve;
  size_t d_len;
  uint8_t d[32];
} cx_ecfp_private_key_t;
