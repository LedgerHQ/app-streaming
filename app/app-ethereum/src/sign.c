#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "app-ethereum.h"
#include "crypto.h"
#include "rlp.h"
#include "sdk.h"
#include "ui.h"

const char *sign(const uint32_t *path,
                 const size_t path_count,
                 const uint8_t *hash,
                 uint8_t *bytes,
                 const size_t max_size,
                 uint16_t *size)
{
    const char *error = NULL;

    uint8_t privkey_data[32];
    if (!derive_node_bip32(CX_CURVE_256K1, path, path_count, privkey_data, NULL)) {
        error = "path derivation failed";
        goto end;
    }

    cx_ecfp_private_key_t privkey;
    ecfp_init_private_key(CX_CURVE_256K1, privkey_data, sizeof(privkey_data), &privkey);

    *size = ecdsa_sign(&privkey, CX_RND_RFC6979 | CX_LAST, CX_SHA256, hash, bytes, max_size);
    if (*size == 0) {
        error = "ecdsa_sign failed";
        goto end;
    }

end:
    explicit_bzero(privkey_data, sizeof(privkey_data));
    explicit_bzero(&privkey, sizeof(privkey));

    return error;
}
