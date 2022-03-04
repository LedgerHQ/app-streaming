#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "app-ethereum.h"
#include "crypto.h"
#include "eip712.h"
#include "sdk.h"
#include "ui.h"

static const char *sign_helper(const RequestSignEip712 *req,
                               const uint8_t *hash,
                               ResponseSignEip712_signature_t *signature)
{
    return sign(req->path, req->path_count, hash, signature->bytes, sizeof(signature->bytes),
                &signature->size);
}

const char *handle_sign_eip712(const RequestSignEip712 *req, ResponseSignEip712 *response)
{
    const char *error = NULL;

    app_loading_start("Parsing message...");

    /* ensure that the message is a null terminated string */
    size_t len = strnlen(req->message, sizeof(req->message));
    if (len >= sizeof(req->message)) {
        return "invalid messsage";
    }

    uint8_t struct_digest[32];
    error = eip712_hash_struct(req->message, len, req->domain_separator, struct_digest);
    if (error != NULL) {
        goto end;
    }

    ctx_sha3_t ctx;
    uint8_t hash[32];

    sha3_256_init(&ctx);
    sha3_256_update(&ctx, (uint8_t *)"\x19\x01", 2);
    sha3_256_update(&ctx, req->domain_separator, sizeof(req->domain_separator));
    sha3_256_update(&ctx, struct_digest, sizeof(struct_digest));
    sha3_256_final(&ctx, hash);

    error = sign_helper(req, hash, &response->signature);
    if (error != NULL) {
        goto end;
    }

end:

    return error;
}
