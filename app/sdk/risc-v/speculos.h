#pragma once

/*
 * Don't add speculos as a requirement only for a few defines... (which might
 * be temporary hopefully, since a nice RISC-V SDK may be developed.)
 */

#define CX_MASK_RND     (7 << 9)
#define CX_RND_PRNG     (1 << 9)
#define CX_RND_TRNG     (2 << 9)
#define CX_RND_RFC6979  (3 << 9)
#define CX_RND_PROVIDED (4 << 9)

#define CX_LAST (1 << 0)

typedef enum cx_curve_e {
    CX_CURVE_SECP256K1 = 0x21,
} cx_curve_t;

enum cx_md_e {
    CX_NONE = 0,
    CX_RIPEMD160 = 1,
    CX_SHA224 = 2,
    CX_SHA256 = 3,
    CX_SHA384 = 4,
    CX_SHA512 = 5,
    CX_KECCAK = 6,
    CX_SHA3 = 7,
    CX_GROESTL = 8,
    CX_BLAKE2B = 9,
    CX_SHAKE128 = 10,
    CX_SHAKE256 = 11,
    CX_SHA3_256 = 12,
    CX_SHA3_512 = 13,
};

typedef enum cx_md_e cx_md_t;

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
