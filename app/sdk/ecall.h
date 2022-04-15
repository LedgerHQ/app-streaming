#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "api/ecall-params.h"
#include "api/uint256.h"

#include "speculos.h"

__attribute__((noreturn)) void ecall_exit(int status);
__attribute__((noreturn)) void ecall_fatal(char *msg);

void ecall_app_loading_start(const char *status);
bool ecall_app_loading_stop(void);
void ecall_bagl_draw_with_context(packed_bagl_component_t *component,
                                  const void *context,
                                  unsigned short context_length,
                                  unsigned char context_encoding);
void ecall_bagl_hal_draw_bitmap_within_rect(int x,
                                            int y,
                                            unsigned int width,
                                            unsigned int height,
                                            const unsigned int *colors,
                                            unsigned int bit_per_pixel,
                                            const unsigned char *bitmap,
                                            unsigned int bitmap_length_bits);
bool ecall_derive_node_bip32(cx_curve_t curve,
                             const unsigned int *path,
                             size_t path_count,
                             uint8_t *private_key,
                             uint8_t *chain);
size_t ecall_ecdsa_sign(const cx_ecfp_private_key_t *key,
                        const int mode,
                        const cx_md_t hash_id,
                        const uint8_t *hash,
                        uint8_t *sig,
                        size_t sig_len);
bool ecall_ecdsa_verify(const cx_ecfp_public_key_t *key,
                        const uint8_t *hash,
                        const uint8_t *sig,
                        const size_t sig_len);
bool ecall_cx_ecfp_generate_pair(cx_curve_t curve,
                                 cx_ecfp_public_key_t *pubkey,
                                 cx_ecfp_private_key_t *privkey,
                                 bool keep_privkey);
bool ecall_hash_final(const cx_hash_id_t hash_id, ctx_hash_guest_t *ctx, uint8_t *digest);
bool ecall_hash_update(const cx_hash_id_t hash_id,
                       ctx_hash_guest_t *ctx,
                       const uint8_t *buffer,
                       const size_t size);
bool ecall_multm(uint8_t *r, const uint8_t *a, const uint8_t *b, const uint8_t *m, size_t len);
void ecall_screen_update(void);
bool ecall_tostring256(const uint256_t *number, const unsigned int base, char *out, size_t len);
void ecall_ux_idle(void);
size_t ecall_xrecv(uint8_t *buffer, size_t size);
void ecall_xsend(const uint8_t *buffer, size_t size);
int ecall_wait_button(void);
