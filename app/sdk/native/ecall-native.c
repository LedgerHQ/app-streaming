#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "ecall.h"
#include "sdk/sdk.h"

void readall(int fd, void *buf, size_t count)
{
	uint8_t *p;
	ssize_t n;
	size_t i;

	p = buf;
	i = count;

	while (i > 0) {
		n = read(fd, p, i);
		if (n == 0) {
            err(1, "readall: read failed");
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

void ecall_xsend(const uint8_t *buffer, size_t size)
{
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
    return 1;
}

void sha256sum(const uint8_t *buffer, size_t size, uint8_t *digest) \
    __attribute__((alias("ecall_sha256sum")));

void screen_update(void) \
    __attribute__((alias("ecall_screen_update")));

void xsend(const uint8_t *buffer, size_t size) \
    __attribute__((alias("ecall_xsend")));

int wait_button(void) \
    __attribute__((alias("ecall_wait_button")));
