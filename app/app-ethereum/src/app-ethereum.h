#pragma once

#include <stdbool.h>
#include <stdlib.h>

#include "crypto.h"
#include "sdk.h"

#include "message.pb.h"

char *handle_get_pubkey(RequestGetPubKey *req, ResponseGetPubKey *response);

void getEthAddressStringFromKey(cx_ecfp_public_key_t *publicKey, char *out, uint64_t chainId);
