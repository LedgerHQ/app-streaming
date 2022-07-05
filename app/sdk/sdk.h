#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "api/uint256.h"
#include "ecall.h"

size_t xrecv(uint8_t *buffer, size_t size);
void xrecvall(uint8_t *buffer, size_t size);

static inline __attribute__((noreturn)) void fatal(char *msg)
{
    ecall_fatal((uint8_t *)msg, strlen(msg));
}

static inline void *xmalloc(size_t size)
{
    void *p = malloc(size);
    if (p == NULL) {
        fatal("malloc failed");
    }
    return p;
}

static inline bool tostring256(const uint256_t *number,
                               const unsigned int base,
                               char *out,
                               size_t len)
{
    return ecall_tostring256(number, base, out, len);
}

static inline void app_loading_start(const char *status)
{
    return ecall_app_loading_start(status);
}

static inline bool app_loading_stop(void)
{
    return ecall_app_loading_stop();
}

static inline void screen_update(void)
{
    ecall_screen_update();
}

static inline void xsend(const uint8_t *buffer, size_t size)
{
    ecall_xsend(buffer, size);
}

static inline int wait_button(void)
{
    return ecall_wait_button();
}

static inline void ux_idle(void)
{
    ecall_ux_idle();
}

struct bagl_component_s;

void bagl_draw_with_context(const struct bagl_component_s *component,
                            const void *context,
                            unsigned short context_length,
                            unsigned char context_encoding);

void bagl_hal_draw_bitmap_within_rect(int x,
                                      int y,
                                      unsigned int width,
                                      unsigned int height,
                                      unsigned int color_count,
                                      const unsigned int *colors,
                                      unsigned int bit_per_pixel,
                                      const unsigned char *bitmap,
                                      unsigned int bitmap_length_bits);
