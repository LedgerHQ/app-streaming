#pragma once

#include <stddef.h>
#include <stdint.h>

void xsend(const uint8_t *buffer, size_t size);
size_t xrecv(uint8_t *buffer, size_t size);

void sha256sum(const uint8_t *buffer, size_t size, uint8_t *digest);
void ux_rectangle(uint32_t color, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void screen_update(void);
int wait_button(void);
