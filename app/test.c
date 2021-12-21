#include <stdio.h>
#include <stdint.h>
#include <stddef.h>


/*int _write(int handle, char *data, int size )
{
    int count ;

    handle = handle ; // unused

    for( count = 0; count < size; count++)
    {
        //outputByte( data[count] ) ;  // Your low-level output function here.
    }

    return count;
    }*/


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
    printf("BLAH %s\n", "kikoo");
    //puts("BLAH %s\n");
    _exit(0);
    return 0;
}
