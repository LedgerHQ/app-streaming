#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void xsend(const uint8_t *buffer, size_t size);
size_t xrecv(uint8_t *buffer, size_t size);
void xrecvall(uint8_t *buffer, size_t size);

void sha256sum(const uint8_t *buffer, size_t size, uint8_t *digest);
void screen_update(void);
int wait_button(void);

void app_loading_start(void);
bool app_loading_stop(void);
