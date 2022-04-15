#include <stdbool.h>

#include "btchip_helpers.h"

#define SCRATCH_SIZE 21

size_t btchip_convert_hex_amount_to_displayable(const uint8_t *amount, uint8_t *out)
{
    const unsigned char LOOP1 = 15;
    const unsigned char LOOP2 = 6;
    /*if (!(config_flag & FLAG_PEERCOIN_UNITS)) {
        LOOP1 = 13;
        LOOP2 = 8;
        } else {*/
    //}
    unsigned short scratch[SCRATCH_SIZE] = { 0 };
    unsigned char offset = 0;
    unsigned char nonZero = 0;
    unsigned char targetOffset = 0;
    unsigned char workOffset;
    unsigned char nscratch = SCRATCH_SIZE;
    unsigned char smin = nscratch - 2;
    unsigned char comma = 0;

    for (size_t i = 0; i < 8; i++) {
        for (size_t j = 0; j < 8; j++) {
            uint8_t k;
            size_t shifted_in = (((amount[i] & 0xff) & ((1 << (7 - j)))) != 0) ? 1 : 0;
            for (k = smin; k < nscratch; k++) {
                scratch[k] += ((scratch[k] >= 5) ? 3 : 0);
            }
            if (scratch[smin] >= 8) {
                smin -= 1;
            }
            for (k = smin; k < nscratch - 1; k++) {
                scratch[k] = ((scratch[k] << 1) & 0xF) | ((scratch[k + 1] >= 8) ? 1 : 0);
            }
            scratch[nscratch - 1] = ((scratch[nscratch - 1] << 1) & 0x0F) |
                                    (shifted_in == 1 ? 1 : 0);
        }
    }

    for (size_t i = 0; i < LOOP1; i++) {
        if (!nonZero && (scratch[offset] == 0)) {
            offset++;
        } else {
            nonZero = 1;
            out[targetOffset++] = scratch[offset++] + '0';
        }
    }

    if (targetOffset == 0) {
        out[targetOffset++] = '0';
    }

    workOffset = offset;
    for (size_t i = 0; i < LOOP2; i++) {
        bool allZero = 1;
        size_t j;
        for (j = i; j < LOOP2; j++) {
            if (scratch[workOffset + j] != 0) {
                allZero = 0;
                break;
            }
        }
        if (allZero) {
            break;
        }
        if (!comma) {
            out[targetOffset++] = '.';
            comma = 1;
        }
        out[targetOffset++] = scratch[offset++] + '0';
    }

    return targetOffset;
}
