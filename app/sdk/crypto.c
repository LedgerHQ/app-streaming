#include <string.h>

#include "crypto.h"

void ecfp_init_private_key(cx_curve_t curve, const uint8_t *raw_key, size_t key_len, cx_ecfp_private_key_t *key)
{
    key->curve = curve;

    if (raw_key != NULL) {
        key->d_len = key_len;
        memcpy(key->d, raw_key, key_len);
    }
}
