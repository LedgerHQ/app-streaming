#pragma once

#include "message.pb-c.h"

#define ALLOC_RESPONSE(type, init) \
    type *response; \
\
    response = malloc(sizeof(*response)); \
    type response_init = init; \
    memcpy(response, &response_init, sizeof(response_init))

ResponseSignTx *handle_sign_tx(MessageSignTx *msg);
