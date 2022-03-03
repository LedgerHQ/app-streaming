#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "app-ethereum.h"
#include "crypto.h"
#include "sdk.h"

#define PREFIX         "\x19\x45thereum Signed Message:\n"
#define MAX_INT32_SIZE (sizeof("4294967295") - 1)

static const char *sign_helper(const RequestSignMsg *req,
                               const uint8_t *hash,
                               ResponseSignMsg_signature_t *signature)
{
    return sign(req->path, req->path_count, hash, signature->bytes, sizeof(signature->bytes),
                &signature->size);
}

const char *handle_sign_message(const RequestSignMsg *req, ResponseSignMsg *response)
{
    const char *error = NULL;

    /* TODO: add UI */

    size_t size = sizeof(PREFIX) - 1 + MAX_INT32_SIZE + req->message.size;
    uint8_t *message = malloc(size);
    if (message == NULL) {
        error = "malloc failed";
        goto end;
    }

    size_t len = snprintf((char *)message, size, PREFIX "%u", req->message.size);
    memcpy(message + len, req->message.bytes, req->message.size);

    uint8_t hash[32];
    sha3_256(message, len + req->message.size, hash);

    error = sign_helper(req, hash, &response->signature);
    if (error != NULL) {
        goto end;
    }

end:
    free(message);
    return error;
}
