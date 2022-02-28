#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "crypto.h"
#include "ecall.h"
#include "ecall-common.h"
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
    const char *p;
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

void ecall_sha256sum(const uint8_t *buffer, size_t size, uint8_t *digest)
{
    /* XXX */
    memset(digest, 'a', 32);
}

void ecall_screen_update(void)
{
}

void ecall_bagl_draw_with_context(packed_bagl_component_t *component, const void *context, unsigned short context_length, unsigned char context_encoding)
{
}

void ecall_bagl_hal_draw_bitmap_within_rect(int x, int y, unsigned int width, unsigned int height, const unsigned int * colors, unsigned int bit_per_pixel, const unsigned char * bitmap, unsigned int bitmap_length_bits)
{
}

int ecall_wait_button(void)
{
    /* XXX */
    return 1;
}

__attribute__((noreturn)) void ecall_fatal(char *msg)
{
    fprintf(stderr, "fatal error: %s\n", msg);
    exit(1);
}

static bool app_loading;

void ecall_app_loading_start(void)
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

void ecall_sha3_256(const uint8_t *buffer, size_t size, uint8_t *digest)
{
    cx_sha3_t ctx;

    cx_keccak_init(&ctx, 256);
    sys_cx_hash((cx_hash_t *)&ctx, CX_LAST, buffer, size, digest, 32);
}

cx_err_t ecall_ecfp_generate_pair(cx_curve_t curve, cx_ecfp_public_key_t *pubkey, cx_ecfp_private_key_t *privkey)
{
    bool keep_private = (privkey != NULL);
    sys_cx_ecfp_generate_pair(curve, pubkey, privkey, keep_private);
    /* XXX */
    return CX_OK;
}

cx_err_t ecall_derive_node_bip32(cx_curve_t curve, const unsigned int *path, size_t path_count, uint8_t *private_key, uint8_t *chain)
{
    sys_os_perso_derive_node_bip32(curve, path, path_count, private_key, chain);
    /* XXX */
    return CX_OK;
}
