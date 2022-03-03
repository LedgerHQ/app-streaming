#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rlp.h"
#include "sdk.h"

static uint64_t rlp_decode_fixed_int(const uint8_t *data, const size_t size)
{
    uint64_t value = 0;

    for (size_t i = 0; i < size; i++) {
        value |= data[i] << (8 * (size - 1 - i));
    }

    return value;
}

/* return 0 on error */
static size_t rlp_decode_int(const uint8_t *data, const size_t size, uint64_t *result)
{
    if (size == 0) {
        return 0;
    } else if (data[0] < 0x80) {
        *result = data[0];
        return 1;
    }

    size_t length_size = data[0] - 0x80;
    if (size < 2 || length_size > 8 || size - 1 < length_size) {
        return 0;
    }

    *result = rlp_decode_fixed_int(&data[1], length_size);

    return 1 + length_size;
}

/* return 0 on error */
static size_t rlp_decode_bytes(const uint8_t *data,
                               const size_t size,
                               uint8_t **result,
                               size_t *result_size)
{
    size_t length_size, string_size;

    if (size == 0 || data[0] >= 0xc0) {
        return 0;
    }

    if (data[0] < 0x80) {
        length_size = 0;
        string_size = 1;
    } else if (data[0] < 0xb8) {
        length_size = 0;
        string_size = data[0] - 0x80;
    } else {
        length_size = data[0] - 0xb7;
        if (length_size > size - 1) {
            return 0;
        }
        string_size = rlp_decode_fixed_int(&data[1], length_size);
    }

    if (1 + length_size + string_size > size) {
        return 0;
    }

    *result = malloc(string_size);
    if (*result == NULL) {
        fatal("malloc");
    }

    if (data[0] < 0x80) {
        (*result)[0] = data[0];
    } else {
        memcpy(*result, &data[1], string_size);
    }

    *result_size = string_size;

    return 1 + length_size + string_size;
}

static void convertUint256BE(const uint8_t *data, const size_t length, uint256_t *target)
{
    uint8_t tmp[INT256_LENGTH];

    memset(tmp, 0, sizeof(tmp) - length);
    memcpy(tmp + sizeof(tmp) - length, data, length);

    readu256BE(tmp, target);
}

/* return 0 on error */
static size_t rlp_decode_uint256(const uint8_t *data, const size_t size, uint256_t *result)
{
    size_t length_size, string_size;

    if (size == 0 || data[0] > 0xb7) {
        return 0;
    }

    if (data[0] < 0x80) {
        length_size = 0;
        string_size = 1;
        convertUint256BE(data, 1, result);
    } else {
        length_size = 0;
        string_size = data[0] - 0x80;

        if (string_size > INT256_LENGTH) {
            return 0;
        }

        convertUint256BE(&data[1], string_size, result);
    }

    return 1 + length_size + string_size;
}

bool rlp_decode_list(const uint8_t *data, const size_t size, struct tx_s *tx)
{
    if (size == 0 || data[0] < 0xc0) {
        return false;
    }

    size_t length_size;
    uint64_t list_size;
    if (data[0] >= 0xc0 && data[0] <= 0xf7) {
        list_size = data[0] - 0xc0;
        length_size = 0;
    } else {
        length_size = data[0] - 0xf8;
        if (length_size > size - 1) {
            return false;
        }

        list_size = rlp_decode_fixed_int(&data[1], length_size);
    }

    if (1 + length_size + list_size != size) {
        return false;
    }

    const uint8_t *p = data + 1 + length_size;
    size_t rem_size = size - (1 + length_size);

    size_t ret;
    ret = rlp_decode_uint256(p, rem_size, &tx->nonce);
    if (ret == 0) {
        return false;
    }
    rem_size -= ret;
    p += ret;

    ret = rlp_decode_uint256(p, rem_size, &tx->gas_price);
    if (ret == 0) {
        return false;
    }
    rem_size -= ret;
    p += ret;

    ret = rlp_decode_uint256(p, rem_size, &tx->gas_limit);
    if (ret == 0) {
        return false;
    }
    rem_size -= ret;
    p += ret;

    size_t to_size;
    ret = rlp_decode_bytes(p, rem_size, (uint8_t **)&tx->to, &to_size);
    if (ret == 0 || to_size != 20) {
        return false;
    }
    rem_size -= ret;
    p += ret;

    ret = rlp_decode_uint256(p, rem_size, &tx->value);
    if (ret == 0) {
        return false;
    }
    rem_size -= ret;
    p += ret;

    ret = rlp_decode_bytes(p, rem_size, &tx->data, &tx->data_size);
    if (ret == 0) {
        return false;
    }
    rem_size -= ret;
    p += ret;

    ret = rlp_decode_int(p, rem_size, &tx->chain_id);
    if (ret == 0) {
        return false;
    }
    rem_size -= ret;
    p += ret;

    uint64_t zero;
    ret = rlp_decode_int(p, rem_size, &zero);
    if (ret == 0 || zero != 0) {
        return false;
    }
    rem_size -= ret;
    p += ret;

    ret = rlp_decode_int(p, rem_size, &zero);
    if (ret == 0 || zero != 0) {
        return false;
    }
    rem_size -= ret;

    if (rem_size != 0) {
        return false;
    }
}
