#include "lfsr.h"

static uint16_t lfsr_state;

/* https://en.wikipedia.org/wiki/Linear-feedback_shift_register#Xorshift_LFSRs */
uint16_t lfsr_get_random(void)
{
    uint16_t lfsr = lfsr_state;

    lfsr ^= lfsr >> 7;
    lfsr ^= lfsr << 9;
    lfsr ^= lfsr >> 13;

    lfsr_state = lfsr;

    return lfsr;
}

void lfsr_init(void)
{
    /* Any nonzero start state will work. */
    lfsr_state = 0xACE1u;
}
