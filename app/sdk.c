#include "sdk.h"

int puts(const char *str)
{
    asm (
         "li t0, 1\n"
         "add a0, %0, 0\n"
         "ecall\n"
         :: "r"(str) : "t0", "a0"
         );

    return 0;
}

void xsend(const uint8_t *buffer, size_t size)
{
    register uint32_t a0 asm ("a0") = (uint32_t)buffer;
    register uint32_t a1 asm ("a1") = (uint32_t)size;

    asm (
         "li t0, 2\n"
         "ecall\n"
         :: "r"(a0), "r"(a1) : "t0", "memory"
         );
}

size_t xrecv(uint8_t *buffer, size_t size)
{
    register uint32_t a0 asm ("a0") = (uint32_t)buffer;
    register uint32_t a1 asm ("a1") = (uint32_t)size;
    size_t ret;

    asm (
         "li t0, 3\n"
         "ecall\n"
         "add %0, a0, 0\n"
         : "=r"(ret) : "r"(a0), "r"(a1) : "t0", "memory"
         );

    return ret;
}

void sha256sum(const uint8_t *buffer, size_t size, uint8_t *digest)
{
    register uint32_t a0 asm ("a0") = (uint32_t)buffer;
    register uint32_t a1 asm ("a1") = (uint32_t)size;
    register uint32_t a2 asm ("a2") = (uint32_t)digest;

    asm (
         "li t0, 4\n"
         "ecall\n"
         :: "r"(a0), "r"(a1), "r"(a2) : "t0", "memory"
         );

}

void ux_rectangle(uint32_t color, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    register uint32_t a0 asm ("a0") = (uint32_t)color;
    register uint32_t a1 asm ("a1") = (uint32_t)x;
    register uint32_t a2 asm ("a2") = (uint32_t)y;
    register uint32_t a3 asm ("a3") = (uint32_t)width;
    register uint32_t a4 asm ("a4") = (uint32_t)height;

    asm (
         "li t0, 6\n"
         "ecall\n"
         :: "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4) : "t0"
         );
}

void screen_update(void)
{
    asm (
         "li t0, 7\n"
         "ecall\n"
         ::: "t0"
        );
}

void bagl_hal_draw_rect(unsigned int color, int x, int y, unsigned int width, unsigned int height)
{
    register uint32_t a0 asm ("a0") = (uint32_t)color;
    register uint32_t a1 asm ("a1") = (uint32_t)x;
    register uint32_t a2 asm ("a2") = (uint32_t)y;
    register uint32_t a3 asm ("a3") = (uint32_t)width;
    register uint32_t a4 asm ("a4") = (uint32_t)height;

    asm (
         "li t0, 6\n"
         "ecall\n"
         :: "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4) : "t0"
        );
}

void bagl_hal_draw_bitmap_within_rect(int x, int y, unsigned int width, unsigned int height, unsigned int color_count, const unsigned int * colors, unsigned int bit_per_pixel, const unsigned char * bitmap, unsigned int bitmap_length_bits)
{
    register uint32_t a0 asm ("a0") = (uint32_t)x;
    register uint32_t a1 asm ("a1") = (uint32_t)y;
    register uint32_t a2 asm ("a2") = (uint32_t)width;
    register uint32_t a3 asm ("a3") = (uint32_t)height;
    register uint32_t a4 asm ("a4") = (uint32_t)colors;
    register uint32_t a5 asm ("a5") = (uint32_t)bit_per_pixel;
    register uint32_t a6 asm ("a6") = (uint32_t)bitmap;
    register uint32_t a7 asm ("a7") = (uint32_t)bitmap_length_bits;

    asm volatile (
         "li t0, 8\n"
         "ecall\n"
         :: "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7) : "t0"
                  );
}
