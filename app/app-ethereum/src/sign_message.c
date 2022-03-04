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
    /* TODO: add UI */

    char prefix[sizeof(PREFIX) - 1 + MAX_INT32_SIZE];
    size_t len = snprintf(prefix, sizeof(prefix), PREFIX "%u", req->message.size);

    app_loading_start("Signing message...");

    ctx_sha3_t ctx;
    uint8_t hash[32];

    sha3_256_init(&ctx);
    sha3_256_update(&ctx, (uint8_t *)prefix, len);
    sha3_256_update(&ctx, req->message.bytes, req->message.size);
    sha3_256_final(&ctx, hash);

    const char *error = sign_helper(req, hash, &response->signature);
    if (error != NULL) {
        goto end;
    }

end:
    return error;
}
