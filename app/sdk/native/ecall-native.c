#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "crypto.h"
#include "ecall-vm.h"
#include "ecall.h"
#include "sdk/sdk.h"

static void readall(int fd, void *buf, size_t count)
{
    uint8_t *p;
    ssize_t n;
    size_t i;

    p = buf;
    i = count;

    while (i > 0) {
        n = read(fd, p, i);
        if (n == 0) {
            errx(1, "readall: connection closed");
        } else if (n == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                err(1, "readall: read failed");
            }
        }
        i -= n;
        p += n;
    }
}

static void writeall(const uint8_t *buffer, size_t size)
{
    const uint8_t *p;
    ssize_t i;

    p = buffer;
    while (size > 0) {
        i = write(STDOUT_FILENO, p, size);
        if (i == 0) {
            err(1, "ecall_xsend: write failed");
        } else if (i == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                err(1, "ecall_xsend: write failed");
            }
        }
        size -= i;
        p += i;
    }
}

bool copy_guest_buffer(guest_pointer_t p_src, void *buf, size_t size)
{
    memcpy(buf, (void *)p_src.addr, size);

    return true;
}

bool copy_host_buffer(guest_pointer_t p_dst, void *buf, size_t size)
{
    memcpy((void *)p_dst.addr, buf, size);

    return true;
}

uint8_t *get_buffer(const uintptr_t addr, const size_t size, const bool writeable)
{
    return (uint8_t *)addr;
}

void ecall_xsend(const uint8_t *buffer, size_t size)
{
    uint32_t n = size;

    writeall((uint8_t *)&n, sizeof(n));
    writeall(buffer, size);
}

size_t ecall_xrecv(uint8_t *buffer, size_t size)
{
    uint32_t n;

    readall(STDIN_FILENO, &n, sizeof(n));

    if (n > size) {
        err(1, "xrecv: invalid size");
    }

    readall(STDIN_FILENO, buffer, n);

    return n;
}

void ecall_screen_update(void)
{
}

void ecall_bagl_draw_with_context(packed_bagl_component_t *component,
                                  const void *context,
                                  unsigned short context_length,
                                  unsigned char context_encoding)
{
}

void ecall_bagl_hal_draw_bitmap_within_rect(int x,
                                            int y,
                                            unsigned int width,
                                            unsigned int height,
                                            const unsigned int *colors,
                                            unsigned int bit_per_pixel,
                                            const unsigned char *bitmap,
                                            unsigned int bitmap_length_bits)
{
}

int ecall_wait_button(void)
{
    /* XXX */
    return 1;
}

__attribute__((noreturn)) void ecall_fatal(uint8_t *msg, size_t size)
{
    fprintf(stderr, "fatal error: ");
    write(STDERR_FILENO, msg, size);
    fprintf(stderr, "\n");
    exit(1);
}

static bool app_loading;

void ecall_app_loading_start(const char *status)
{
    app_loading = true;
}

bool ecall_app_loading_stop(void)
{
    bool prev = app_loading;

    app_loading = false;

    return prev;
}

void ecall_ux_idle(void)
{
}
