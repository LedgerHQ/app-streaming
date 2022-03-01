#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app-ethereum.h"
#include "crypto.h"
#include "ui.h"
#include "sdk.h"

struct tx_s {
    txInt256_t nonce;
    txInt256_t gas_price;
    txInt256_t gas_limit;
    uint8_t *to;
    txInt256_t value;
    uint8_t *data;
    size_t data_size;
    uint64_t chain_id;
};

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
static size_t rlp_decode_bytes(const uint8_t *data, const size_t size, uint8_t **result, size_t *result_size)
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

    if (1 + length_size + string_size > size)  {
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

/* return 0 on error */
static size_t rlp_decode_uint256(const uint8_t *data, const size_t size, txInt256_t *result)
{
    size_t length_size, string_size;

    if (size == 0 || data[0] > 0xb7) {
        return 0;
    }

    if (data[0] < 0x80) {
        length_size = 0;
        string_size = 1;
        result->value[0] = data[0];
    } else {
        length_size = 0;
        string_size = data[0] - 0x80;

        if (string_size > sizeof(result->value))  {
            return 0;
        }
        memcpy(result->value, &data[1], string_size);
    }

    result->length = string_size;

    return 1 + length_size + string_size;
}


static bool rlp_decode_list(const uint8_t *data, const size_t size, struct tx_s *tx)
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

static const char *sign(const uint32_t *path, const size_t path_count, const uint8_t *hash, ResponseSignTx_signature_t *signature)
{
    const char *error = NULL;

    uint8_t privkey_data[32];
    if (derive_node_bip32(CX_CURVE_256K1, path, path_count, privkey_data, NULL) != CX_OK) {
        error = "path derivation failed";
        goto end;
    }

    cx_ecfp_private_key_t privkey;
    ecfp_init_private_key(CX_CURVE_256K1, privkey_data, sizeof(privkey_data), &privkey);

    signature->size = ecdsa_sign(&privkey, CX_RND_RFC6979 | CX_LAST, CX_SHA256, hash, signature->bytes, sizeof(signature->bytes));
    if (signature->size == 0) {
        error = "ecdsa_sign failed";
        goto end;
    }

 end:
    explicit_bzero(privkey_data, sizeof(privkey_data));
    explicit_bzero(&privkey, sizeof(privkey));

    return error;
}

const char *handle_sign_tx(const RequestSignTx *req, ResponseSignTx *response)
{
    const char *error = NULL;

    struct tx_s tx;
    tx.to = NULL;
    tx.data = NULL;

    if (rlp_decode_list(req->raw_tx.bytes, req->raw_tx.size, &tx)) {
        error = "rlp decoding failed";
        goto end;
    }

    ui_set_tx_address(tx.chain_id, tx.to);
    ui_set_tx_amount(tx.chain_id, &tx.value);
    ui_set_tx_network_name(tx.chain_id);
    ui_set_tx_fees(tx.chain_id, &tx.gas_price, &tx.gas_limit);
    response->approved = ui_sign_tx_validation();
    if (!response->approved) {
        goto end;
    }

    uint8_t hash[32];
    sha3_256(req->raw_tx.bytes, req->raw_tx.size, hash);

    error = sign(req->path, req->path_count, hash, &response->signature);
    if (error != NULL) {
        goto end;
    }

 end:
    free(tx.to);
    free(tx.data);

    return NULL;
}
