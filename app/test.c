static void hello(char *str)
{
    asm("ecall");
}

int main(void)
{
    hello("blah");
    return 0;
}
