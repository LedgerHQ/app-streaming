#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "crypto.h"
#include "sdk.h"
#include "uint256.h"

#define ADDRESS_SIZE   40
#define WEI_TO_ETHER   18
#define MAX_TICKER_LEN 12

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static const char HEXDIGITS[] = "0123456789abcdef";

static void hexdump(uint8_t *src, uint8_t *dst, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        uint8_t digit = src[i];
        dst[2 * i] = HEXDIGITS[(digit >> 4) & 0x0f];
        dst[2 * i + 1] = HEXDIGITS[digit & 0x0f];
    }
}

static void checksum(const uint8_t *hash_checksum, const uint8_t *hash_address, char *out)
{
    for (size_t i = 0; i < ADDRESS_SIZE; i++) {
        uint8_t digit = hash_address[i / 2];
        if ((i % 2) == 0) {
            digit = (digit >> 4) & 0x0f;
        } else {
            digit = digit & 0x0f;
        }
        if (digit < 10) {
            out[i] = HEXDIGITS[digit];
        } else {
            int v = (hash_checksum[i / 2] >> (4 * (1 - i % 2))) & 0x0f;
            if (v >= 8) {
                out[i] = HEXDIGITS[digit] - 'a' + 'A';
            } else {
                out[i] = HEXDIGITS[digit];
            }
        }
    }

    out[ADDRESS_SIZE] = '\0';
}

void getEthAddressStringFromBinary(uint8_t *hash_address, char *out, uint64_t chain_id)
{
    uint8_t hash_checksum[INT256_LENGTH];

    if (chain_id == 30 || chain_id == 31) {
        /* EIP-1191 */
        uint8_t tmp[20 + 2 + ADDRESS_SIZE + 1];
        size_t offset = snprintf((char *)tmp, sizeof(tmp), "%" PRIu64 "0x", chain_id);
        hexdump(hash_address, &tmp[offset], 20);
        sha3_256(tmp, offset + ADDRESS_SIZE, hash_checksum);
    } else {
        uint8_t tmp[ADDRESS_SIZE];

        hexdump(hash_address, tmp, 20);
        sha3_256(tmp, ADDRESS_SIZE, hash_checksum);
    }

    checksum(hash_checksum, hash_address, out);
}

void getEthAddressStringFromKey(cx_ecfp_public_key_t *publicKey, char *out, uint64_t chain_id)
{
    uint8_t hashAddress[INT256_LENGTH];
    sha3_256(publicKey->W + 1, 64, hashAddress);
    getEthAddressStringFromBinary(hashAddress + 12, out, chain_id);
}

#define MAX_NETWORK_TICKER_LEN 8

typedef struct network_info_s {
    const char *name;
    const char *ticker;
    const uint64_t chain_id;
} network_info_t;

static const network_info_t NETWORK_MAPPING[] = {
    { .chain_id = 1, .name = "Ethereum", .ticker = "ETH" },
    { .chain_id = 3, .name = "Ropsten", .ticker = "ETH" },
    { .chain_id = 4, .name = "Rinkeby", .ticker = "ETH" },
    { .chain_id = 5, .name = "Goerli", .ticker = "ETH" },
    { .chain_id = 10, .name = "Optimism", .ticker = "ETH" },
    { .chain_id = 42, .name = "Kovan", .ticker = "ETH" },
    { .chain_id = 56, .name = "BSC", .ticker = "BNB" },
    { .chain_id = 100, .name = "xDai", .ticker = "xDAI" },
    { .chain_id = 137, .name = "Polygon", .ticker = "MATIC" },
    { .chain_id = 250, .name = "Fantom", .ticker = "FTM" },
    { .chain_id = 42161, .name = "Arbitrum", .ticker = "AETH" },
    { .chain_id = 42220, .name = "Celo", .ticker = "CELO" },
    { .chain_id = 43114, .name = "Avalanche", .ticker = "AVAX" },
    { .chain_id = 44787, .name = "Celo Alfajores", .ticker = "aCELO" },
    { .chain_id = 62320, .name = "Celo Baklava", .ticker = "bCELO" },
    { .chain_id = 11297108109, .name = "Palm Network", .ticker = "PALM" }
};

static const network_info_t *get_network(uint64_t chain_id)
{
    for (int i = 0; i < sizeof(NETWORK_MAPPING) / sizeof(*NETWORK_MAPPING); i++) {
        if (NETWORK_MAPPING[i].chain_id == chain_id) {
            return &NETWORK_MAPPING[i];
        }
    }

    return NULL;
}

static const char *get_network_name(uint64_t chain_id)
{
    const network_info_t *network = get_network(chain_id);

    return (network != NULL) ? network->name : NULL;
}

static const char *get_network_ticker(uint64_t chain_id)
{
    const network_info_t *network = get_network(chain_id);

    return (network != NULL) ? network->ticker : NULL;
}

void set_network_name(const uint64_t chain_id, char *name, size_t size)
{
    const char *network_name = get_network_name(chain_id);

    if (network_name == NULL) {
        snprintf(name, size, "%" PRIu64, chain_id);
    } else {
        strncpy(name, network_name, size);
        name[size - 1] = '\x00';
    }
}

static void computeFees(const uint256_t *gasPrice, const uint256_t *gasLimit, uint256_t *output)
{
    mul256((uint256_t *)gasPrice, (uint256_t *)gasLimit, output);
}

static bool adjustDecimals(char *src,
                           uint32_t srcLength,
                           char *target,
                           uint32_t targetLength,
                           uint8_t decimals)
{
    uint32_t startOffset;
    uint32_t lastZeroOffset = 0;
    uint32_t offset = 0;
    if ((srcLength == 1) && (*src == '0')) {
        if (targetLength < 2) {
            return false;
        }
        target[0] = '0';
        target[1] = '\0';
        return true;
    }
    if (srcLength <= decimals) {
        uint32_t delta = decimals - srcLength;
        if (targetLength < srcLength + 1 + 2 + delta) {
            return false;
        }
        target[offset++] = '0';
        target[offset++] = '.';
        for (uint32_t i = 0; i < delta; i++) {
            target[offset++] = '0';
        }
        startOffset = offset;
        for (uint32_t i = 0; i < srcLength; i++) {
            target[offset++] = src[i];
        }
        target[offset] = '\0';
    } else {
        uint32_t sourceOffset = 0;
        uint32_t delta = srcLength - decimals;
        if (targetLength < srcLength + 1 + 1) {
            return false;
        }
        while (offset < delta) {
            target[offset++] = src[sourceOffset++];
        }
        if (decimals != 0) {
            target[offset++] = '.';
        }
        startOffset = offset;
        while (sourceOffset < srcLength) {
            target[offset++] = src[sourceOffset++];
        }
        target[offset] = '\0';
    }
    for (uint32_t i = startOffset; i < offset; i++) {
        if (target[i] == '0') {
            if (lastZeroOffset == 0) {
                lastZeroOffset = i;
            }
        } else {
            lastZeroOffset = 0;
        }
    }
    if (lastZeroOffset != 0) {
        target[lastZeroOffset] = '\0';
        if (target[lastZeroOffset - 1] == '.') {
            target[lastZeroOffset - 1] = '\0';
        }
    }
    return true;
}

static void value_to_string(const char *feeTicker,
                            const uint256_t *rawFee,
                            char *displayBuffer,
                            const size_t displayBufferSize)
{
    char buf[100];
    char buf2[100];

    tostring256((uint256_t *)rawFee, 10, buf, sizeof(buf));
    adjustDecimals(buf, strlen(buf), buf2, sizeof(buf2), WEI_TO_ETHER);

    snprintf(displayBuffer, displayBufferSize, "%s %s", feeTicker, buf2);
    displayBuffer[displayBufferSize - 1] = '\x00';
}

void compute_amount(uint64_t chain_id,
                    const uint256_t *amount,
                    char *out_buffer,
                    size_t out_buffer_size)
{
    const char *ticker = get_network_ticker(chain_id);

    value_to_string(ticker, amount, out_buffer, out_buffer_size);
}

void compute_fees(uint64_t chain_id,
                  const uint256_t *gas_price,
                  const uint256_t *gas_limit,
                  char *buffer,
                  const size_t size)
{
    const char *feeTicker = get_network_ticker(chain_id);
    uint256_t rawFee = { 0 };
    computeFees(gas_price, gas_limit, &rawFee);
    value_to_string(feeTicker, &rawFee, buffer, size);
}

static bool allzeroes(void *buf, size_t n)
{
    uint8_t *p = (uint8_t *)buf;
    for (size_t i = 0; i < n; ++i) {
        if (p[i]) {
            return false;
        }
    }
    return true;
}

static bool uint256_to_decimal(const uint8_t *value, size_t value_len, char *out, size_t out_len)
{
    if (value_len > INT256_LENGTH) {
        // value len is bigger than INT256_LENGTH ?!
        return false;
    }

    uint16_t n[16] = { 0 };
    // Copy and right-align the number
    memcpy((uint8_t *)n + INT256_LENGTH - value_len, value, value_len);

    // Special case when value is 0
    if (allzeroes(n, INT256_LENGTH)) {
        if (out_len < 2) {
            // Not enough space to hold "0" and \0.
            return false;
        }
        out[0] = '0';
        out[1] = '\x00';
        return true;
    }

    uint16_t *p = n;
    for (int i = 0; i < 16; i++) {
        n[i] = __builtin_bswap16(*p++);
    }
    int pos = out_len;
    while (!allzeroes(n, sizeof(n))) {
        if (pos == 0) {
            return false;
        }
        pos -= 1;
        unsigned int carry = 0;
        for (int i = 0; i < 16; i++) {
            int rem = ((carry << 16) | n[i]) % 10;
            n[i] = ((carry << 16) | n[i]) / 10;
            carry = rem;
        }
        out[pos] = '0' + carry;
    }
    memmove(out, out + pos, out_len - pos);
    out[out_len - pos] = 0;
    return true;
}

bool amountToString(const uint8_t *amount,
                    uint8_t amount_size,
                    uint8_t decimals,
                    const char *ticker,
                    char *out_buffer,
                    size_t out_buffer_size)
{
    char tmp_buffer[100] = { 0 };

    if (uint256_to_decimal(amount, amount_size, tmp_buffer, sizeof(tmp_buffer)) == false) {
        return false;
    }

    size_t amount_len = strnlen(tmp_buffer, sizeof(tmp_buffer));
    size_t ticker_len = strnlen(ticker, MAX_TICKER_LEN);

    memcpy(out_buffer, ticker, MIN(out_buffer_size, ticker_len));

    if (adjustDecimals(tmp_buffer, amount_len, out_buffer + ticker_len,
                       out_buffer_size - ticker_len - 1, decimals) == false) {
        return false;
    }

    out_buffer[out_buffer_size - 1] = '\0';

    printf("[%s]\n", out_buffer);

    return true;
}
