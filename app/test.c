#include <stdio.h>
#include <stdint.h>
#include <stddef.h>


int puts(const char *str)
{
    asm(
        "li t0, 1\n"
        "add a0, %0, 0\n"
        "ecall\n"
        :: "r"(str) : "t0", "a0"
        );
}

static void _exit(int code)
{
    asm(
        "li t0, 0\n"
        "ecall\n"
        );
}

/* XXX */
double __trunctfdf2 (long double a)
{
    return a;
}

int main(void)
{
    printf("BLAH\n");
    //puts("BLAH %s\n");
    _exit(0);
    return 0;
}
