#pragma once

#include <stddef.h>
#include <stdint.h>

void xsend(const uint8_t *buffer, size_t size);
size_t xrecv(uint8_t *buffer, size_t size);

void sha256sum(const uint8_t *buffer, size_t size, uint8_t *digest);
