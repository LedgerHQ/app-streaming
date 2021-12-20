//#include <stdint.h>

static void hello(char *str)
{
    unsigned int i;
    char c;

    /*for (i = 0; str[i] != '\x00'; i++) {
        c = str[i];
        asm(
            "li t0, 4\n"
            "add a0, %0, 0\n"
            "ecall\n" :: "r"(c) : "t0", "a0");
            }*/
    asm(
        "li t0, 4\n"
        "add a0, %0, 0\n"
        "ecall\n" :: "r"(str) : "t0", "a0");
}

static void _exit(int code)
{
    asm(
        "li t0, 0\n"
        "ecall\n"
        );
}

int main(void)
{
    hello("BLAH\n");
    _exit(0);
    return 0;
}
