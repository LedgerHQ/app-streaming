#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "tx.h"
#include "uint256.h"

static const uint256_t gas_limit = { { 0, 0, 0, 21000 } };

static uint8_t unhex_helper(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else {
        // XXX
        // fatal("invalid hex character");
        return 0;
    }
}

static void unhex(const char *hex, uint8_t *bin, size_t bin_size)
{
    for (size_t i = 0; i < bin_size; i++) {
        uint8_t c;
        c = unhex_helper(hex[i * 2]) << 4;
        c |= unhex_helper(hex[i * 2 + 1]);
        bin[i] = c;
    }
}

bool encode_bytes(const uint8_t *bytes, const size_t size, uint8_t **out, size_t *p_out_size)
{
    if (size == 0) {
        return false;
    }

    uint8_t *p = *out;
    size_t out_size;

    if (size == 1 && bytes[0] < 0x80) {
        if (*p_out_size < 1) {
            return false;
        }

        p[0] = bytes[0];
        out_size = 1;
    } else if (size < 56) {
        if (*p_out_size < 1 + size) {
            return false;
        }

        p[0] = '\x80' + size;
        memcpy(p + 1, bytes, size);

        out_size = 1 + size;
    } else {
        // buffer larger than 55 bytes aren't supported
        return false;
    }

    *out += out_size;
    *p_out_size -= out_size;

    return true;
}

/**
 * Input is a unsigned int encoded in big-endian.
 */
static bool bytes_to_uint256(const uint8_t *bytes, const size_t size, uint256_t *target)
{
    if (size > sizeof(*target)) {
        return false;
    }

    uint8_t tmp[INT256_LENGTH];
    memset(tmp, '\x00', INT256_LENGTH - size);
    memcpy(tmp + sizeof(tmp) - size, bytes, size);

    readu256BE(tmp, target);

    return true;
}

static bool encode_uint256(const uint256_t *n, uint8_t **buffer, size_t *maxsize)
{
    const uint8_t *p = (const uint8_t *)n;

    uint8_t tmp[INT256_LENGTH];
    readu256BE((uint8_t *)n, (uint256_t *)tmp);

    // find first non-null byte
    size_t i;
    for (i = 0; i < sizeof(uint256_t); i++) {
        if (tmp[i] != '\x00') {
            break;
        }
    }

    size_t size = sizeof(uint256_t) - i;
    return encode_bytes(tmp + i, size, buffer, maxsize);
}

static bool encode_gas_price(const uint8_t *fees,
                             const size_t fees_size,
                             uint8_t **buffer,
                             size_t *maxsize)
{
    uint256_t n;
    if (!bytes_to_uint256(fees, fees_size, &n)) {
        return false;
    }

    uint256_t gas_price;
    mul256((uint256_t *)&gas_limit, &n, &gas_price);

    return encode_uint256(&gas_price, buffer, maxsize);
}

// gas limit is 21000
static bool encode_gas_limit(uint8_t **buffer, size_t *maxsize)
{
    if (*maxsize < 3) {
        return false;
    }

    memcpy(*buffer, "\x82\x52\x08", 3);
    *buffer += 3;
    *maxsize -= 3;

    return true;
}

// address size: 42 bytes
static bool encode_address(const char *address, uint8_t **buffer, size_t *maxsize)
{
    const char *p = (strncmp(address, "0x", 2) != 0) ? address : address + 2;
    const size_t len = strlen(p);

    uint8_t bin[20];
    if (len != sizeof(bin) * 2) {
        return false;
    }

    unhex(p, bin, sizeof(bin));

    return encode_bytes(bin, sizeof(bin), buffer, maxsize);
}

/**
 * @return 0 on error
 */
size_t rlp_encode_tx(const char *address,
                     const uint8_t *amount,
                     const size_t amount_size,
                     const uint8_t *fees,
                     const size_t fees_size,
                     uint8_t *buffer,
                     const size_t buf_size)
{
    uint8_t *p = buffer;
    size_t size = buf_size;

    /* XXX: nonce  */
    if (size < 6) {
        return 0;
    }
    memcpy(p, "\x85\x03\x06\xdc\x42\x00", 6);
    p += 6;
    size -= 6;

    if (!encode_gas_price(fees, fees_size, &p, &size)) {
        return 0;
    }

    if (!encode_gas_limit(&p, &size)) {
        return 0;
    }

    if (!encode_address(address, &p, &size)) {
        return 0;
    }

    // maximum amount size: 78 bytes
    if (!encode_bytes(amount, amount_size, &p, &size)) {
        return 0;
    }

    /* EIP-255 */
#if 1
    if (size < 4) {
        return 0;
    }

    p[0] = '\x80'; // empty data
    p[1] = '\x01'; // chain id
    p[2] = '\x80'; // 0
    p[3] = '\x80'; // 0

    size -= 4;
#else
    if (size < 1) {
        return 0;
    }

    p[0] = '\x80'; // empty data

    size -= 1;
#endif

    return buf_size - size;
}

/**
 * @return 0 on error
 */
size_t rlp_buffer_to_list(uint8_t *buffer, const size_t size, const size_t max_size)
{
    size_t offset;

    if (size < 56) {
        offset = 1;
    } else if (size < 0x80) {
        offset = 2;
    } else if (size < 0x100) {
        offset = 3;
    } else {
        return 0;
    }

    if (offset + size > max_size) {
        return 0;
    }

    memmove(buffer + offset, buffer, size);

    if (size < 56) {
        buffer[0] = '\xc0' + size;
    } else if (size < 0x80) {
        buffer[0] = '\xf7' + 1;
        buffer[1] = size;
    } else if (size < 0x100) {
        buffer[0] = '\xf7' + 2;
        buffer[1] = '\x80' + 1;
        buffer[2] = size;
    } else {
        return 0;
    }

    return offset + size;
}

size_t eth_create_tx(const uint32_t *path,
                     const size_t path_count,
                     const char *address,
                     const uint8_t *amount,
                     const size_t amount_size,
                     const uint8_t *fees,
                     const size_t fees_size,
                     uint8_t *tx,
                     const size_t max_size)
{
    size_t size = rlp_encode_tx(address, amount, amount_size, fees, fees_size, tx, max_size);
    if (size == 0) {
        return 0;
    }

    uint8_t *p = tx + size;
    const size_t sig_size = sign_tx(path, path_count, tx, size, p, max_size - size);
    if (sig_size == 0) {
        return 0;
    }

    size = rlp_buffer_to_list(tx, size + sig_size, max_size);
    if (size == 0) {
        return 0;
    }

    return size;
}
