#pragma once

#include <stddef.h>
#include <stdint.h>

#include "sdk/api/ecall-params.h"

__attribute__((noreturn)) void ecall_exit(int status);
__attribute__((noreturn)) void fatal(char *msg);
size_t ecall_xrecv(uint8_t *buffer, size_t size);
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

void ecall_xsend(const uint8_t *buffer, size_t size);
size_t ecall_xrecv(uint8_t *buffer, size_t size);

void ecall_sha256sum(const uint8_t *buffer, size_t size, uint8_t *digest);
void ecall_screen_update(void);
int ecall_wait_button(void);
