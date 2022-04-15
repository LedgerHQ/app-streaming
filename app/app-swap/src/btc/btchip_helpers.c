#include <stdio.h>
#include <string.h>

#include "base58.h"
#include "btchip_helpers.h"
#include "crypto.h"

static inline int btchip_encode_base58(const uint8_t *in,
                                       size_t length,
                                       uint8_t *out,
                                       size_t *outlen)
{
    int ret = base58_encode(in, length, (char *)out, *outlen);
    if (ret >= 0) {
        *outlen = ret;
    }
    return ret;
}

void btchip_public_key_hash160(const uint8_t *in, const size_t inlen, uint8_t *out)
{
    uint8_t buffer[32];

    sha256(in, inlen, buffer);
    ripemd160(buffer, 32, out);
}

void btchip_compute_checksum(uint8_t *in, size_t inlen, uint8_t *output)
{
    uint8_t checksumBuffer[32];
    sha256(in, inlen, checksumBuffer);
    sha256(checksumBuffer, 32, checksumBuffer);
    memcpy(output, checksumBuffer, 4);
}

size_t btchip_public_key_to_encoded_base58(const uint8_t *in,
                                           const size_t inlen,
                                           uint8_t *out,
                                           const size_t outlen,
                                           const unsigned short version,
                                           const uint8_t alreadyHashed)
{
    uint8_t tmpBuffer[34];

    uint8_t versionSize = (version > 255 ? 2 : 1);
    size_t outputLen;

    if (!alreadyHashed) {
        btchip_public_key_hash160(in, inlen, tmpBuffer + versionSize);
        if (version > 255) {
            tmpBuffer[0] = (version >> 8);
            tmpBuffer[1] = version;
        } else {
            tmpBuffer[0] = version;
        }
    } else {
        memmove(tmpBuffer, in, 20 + versionSize);
    }

    btchip_compute_checksum(tmpBuffer, 20 + versionSize, tmpBuffer + 20 + versionSize);

    outputLen = outlen;
    if (btchip_encode_base58(tmpBuffer, 24 + versionSize, out, &outputLen) < 0) {
        // THROW(EXCEPTION);
        return 0;
    }

    return outputLen;
}
