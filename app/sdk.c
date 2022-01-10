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
    asm (
         "li t0, 2\n"
         "add a0, %0, 0\n"
         "add a1, %1, 0\n"
         "ecall\n"
         :: "r"(buffer), "r"(size) : "t0", "a0", "a1", "memory"
         );
}

size_t xrecv(uint8_t *buffer, size_t size)
{
    size_t ret;

    asm (
         "li t0, 3\n"
         "add a0, %1, 0\n"
         "add a1, %2, 0\n"
         "ecall\n"
         "add %0, a0, 0\n"
         : "=r"(ret) : "r"(buffer), "r"(size) : "t0", "a0", "a1", "memory"
         );

    return ret;
}

void sha256sum(const uint8_t *buffer, size_t size, uint8_t *digest)
{
    asm (
         "li t0, 4\n"
         "add a0, %0, 0\n"
         "add a1, %1, 0\n"
         "add a2, %2, 0\n"
         "ecall\n"
         :: "r"(buffer), "r"(size), "r"(digest) : "t0", "a0", "a1", "a2", "memory"
         );

}

/* XXX */
double __trunctfdf2 (long double a)
{
    return a;
}
