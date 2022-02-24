#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

void xsend(const uint8_t *buffer, size_t size);
size_t xrecv(uint8_t *buffer, size_t size);
void xrecvall(uint8_t *buffer, size_t size);

void sha256sum(const uint8_t *buffer, size_t size, uint8_t *digest);
void screen_update(void);
int wait_button(void);

void app_loading_start(void);
bool app_loading_stop(void);

void ux_idle(void);

__attribute__((noreturn)) void fatal(char *msg);

static inline void *xmalloc(size_t size)
{
    void *p = malloc(size);
    if (p == NULL) {
        fatal("malloc failed");
    }
    return p;
}
