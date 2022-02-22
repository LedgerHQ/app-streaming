#pragma once

#include <stdlib.h>

#include "crypto.h"
#include "sdk.h"

#include "message.pb-c.h"

#define ALLOC_RESPONSE(type, init) \
    type *response; \
\
    response = xmalloc(sizeof(*response)); \
    type response_init = init; \
    memcpy(response, &response_init, sizeof(response_init))

static inline void protobuf_dup_bytes_helper(ProtobufCBinaryData *bin, const void *src, const size_t size)
{
    bin->data = xmalloc(size);
    bin->len = size;
    memcpy(bin->data, src, size);
}

#define protobuf_dup_bytes(message, field_name, src, size)              \
    do {                                                                \
        message->has_##field_name = true;                               \
        protobuf_dup_bytes_helper(&message->field_name, src, size);     \
    } while (0)

#define protobuf_bool_is_true(message, field_name)      \
    (message->has_##field_name && message->field_name)

#define protobuf_set_bool(message, field_name, value)   \
    do {                                                \
        message->has_##field_name = true;               \
        message->field_name = value;                    \
    } while (0)

ResponseSignTx *handle_sign_tx(RequestSignTx *msg);
ResponseGetPubKey *handle_get_pubkey(RequestGetPubKey *req);

void getEthAddressStringFromKey(cx_ecfp_public_key_t *publicKey, char *out, uint64_t chainId);
