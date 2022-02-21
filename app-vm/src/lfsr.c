#include "lfsr.h"

#define POLY_MASK_32 0xB4BCD35C
#define POLY_MASK_31 0x7A5BC2E3

unsigned int lfsr32, lfsr31;

static int shift_lfsr(unsigned int *lfsr,  unsigned int polymonial_mask)
{
    int feedback;

    feedback = *lfsr & 1;

    *lfsr >>= 1;
    if (feedback == 1) {
        *lfsr ^= polymonial_mask;
    }

    return *lfsr;
}

int get_random(void)
{
    /*this random number generator shifts the 32-bit LFSR twice before XORing
      it with the 31-bit LFSR. the bottom 16 bits are used for the random number*/
    shift_lfsr(&lfsr32, POLY_MASK_32);
    return (shift_lfsr(&lfsr32, POLY_MASK_32) ^ shift_lfsr(&lfsr31, POLY_MASK_31));
}

void init_lfsr(void)
{
    lfsr32 = 0xABCDE; //seed values
    lfsr31 = 0x23456789;
}
