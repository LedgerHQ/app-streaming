static void hello(char *str)
{
    asm(
        "li t0, 1\n"
        "ecall\n");
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
    hello("blah");
    _exit(0);
    return 0;
}
