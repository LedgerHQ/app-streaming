#pragma once

#include <stdbool.h>

#include "crypto.h"
#include "uint256.h"

/*
 * The following functions are aliased to ecall implementations.
 */

void app_loading_start(void) \
    __attribute__((alias("ecall_app_loading_start")));

bool app_loading_stop(void) \
    __attribute__((alias("ecall_app_loading_stop")));

__attribute__((noreturn)) void fatal(char *msg) \
    __attribute__((alias("ecall_fatal")));

void sha256sum(const uint8_t *buffer, size_t size, uint8_t *digest) \
    __attribute__((alias("ecall_sha256sum")));

void screen_update(void) \
    __attribute__((alias("ecall_screen_update")));

void xsend(const uint8_t *buffer, size_t size) \
    __attribute__((alias("ecall_xsend")));

int wait_button(void) \
    __attribute__((alias("ecall_wait_button")));

void sha3_256(const uint8_t *buffer, size_t size, uint8_t *digest) \
    __attribute__((alias("ecall_sha3_256")));

void ux_idle(void) \
    __attribute__((alias("ecall_ux_idle")));

cx_err_t ecfp_generate_pair(cx_curve_t curve, cx_ecfp_public_key_t *pubkey, cx_ecfp_private_key_t *privkey) \
    __attribute__((alias("ecall_ecfp_generate_pair")));

cx_err_t derive_node_bip32(cx_curve_t curve, const unsigned int *path, size_t path_count, uint8_t *private_key, uint8_t *chain) \
    __attribute__((alias("ecall_derive_node_bip32")));

size_t ecdsa_sign(const cx_ecfp_private_key_t *key, const int mode, const cx_md_t hash_id, const uint8_t * hash, uint8_t *sig, size_t sig_len) \
    __attribute__((alias("ecall_ecdsa_sign")));

void mult(uint8_t *r, const uint8_t *a, const uint8_t *b, size_t len) \
    __attribute__((alias("ecall_mult")));

void multm(uint8_t *r, const uint8_t *a, const uint8_t *b, const uint8_t *m, size_t len) \
    __attribute__((alias("ecall_multm")));

bool tostring256(const uint256_t *number, const unsigned int base, char *out, size_t len) \
    __attribute__((alias("ecall_tostring256")));
