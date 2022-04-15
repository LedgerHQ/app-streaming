#include <stdio.h>
#include <string.h>

#include "btc.h"
#include "btchip_helpers.h"
#include "crypto.h"
#include "segwit.h"

#define CHAIN_CODE_SIZE 32

#define P2_LEGACY        0x00 // P2PKH
#define P2_SEGWIT        0x01 // P2SH
#define P2_NATIVE_SEGWIT 0x02 // bech32

#define COIN_P2PKH_VERSION 0
#define COIN_P2SH_VERSION  5

static void btchip_compress_public_key_value(uint8_t *value)
{
    value[0] = ((value[64] & 1) ? 0x03 : 0x02);
}

static bool derive_pubkey(const uint32_t *path,
                          const size_t path_count,
                          cx_ecfp_public_key_t *pubkey,
                          uint8_t *chain_code)
{
    bool success = false;

    uint8_t privkey_data[32];
    if (!derive_node_bip32(CX_CURVE_256K1, path, path_count, privkey_data, chain_code)) {
        goto end;
    }

    cx_ecfp_private_key_t privkey;
    if (!ecfp_generate_keypair(CX_CURVE_256K1, privkey_data, sizeof(privkey_data), pubkey,
                               &privkey)) {
        goto end;
    }

    success = true;

end:
    explicit_bzero(privkey_data, sizeof(privkey_data));
    explicit_bzero(&privkey, sizeof(privkey));

    return success;
}

static bool derive_compressed_public_key(const uint32_t *path,
                                         const size_t path_count,
                                         uint8_t *public_key)
{
    cx_ecfp_public_key_t pubkey;
    uint8_t chain_code[CHAIN_CODE_SIZE];
    if (!derive_pubkey(path, path_count, &pubkey, chain_code)) {
        return false;
    }

    btchip_compress_public_key_value(pubkey.W);

    memcpy(public_key, pubkey.W, 33);

    return true;
}

static bool get_address(const uint8_t format,
                        const uint8_t *compressed_pub_key,
                        char *address,
                        const size_t max_address_length)
{
    size_t address_length;

    if (format == P2_LEGACY) {
        address_length = btchip_public_key_to_encoded_base58(
            compressed_pub_key, 33, (uint8_t *)address, max_address_length - 1, COIN_P2PKH_VERSION,
            0);
        address[address_length] = 0;
    } else if (format == P2_SEGWIT) {
        uint8_t tmp[22];
        tmp[0] = 0x00;
        tmp[1] = 0x14;
        btchip_public_key_hash160(compressed_pub_key, 33, tmp + 2);
        address_length = btchip_public_key_to_encoded_base58(tmp, sizeof(tmp), (uint8_t *)address,
                                                             150, COIN_P2SH_VERSION, 0);
        address[address_length] = 0;
    } else if (format == P2_NATIVE_SEGWIT) {
        uint8_t tmp[20];
        btchip_public_key_hash160(compressed_pub_key, 33, tmp);
        if (!segwit_addr_encode(address, "bc", 0, tmp, sizeof(tmp))) {
            return false;
        }
    } else {
        return false;
    }

    return true;
}

static bool btc_validate_address(const char *addr, const uint32_t *path, const size_t path_count)
{
    uint8_t compressed_public_key[33];
    if (!derive_compressed_public_key(path, path_count, compressed_public_key)) {
        return false;
    }

    uint8_t format;
    if (memcmp(addr, "bc1", 3) == 0) {
        format = P2_NATIVE_SEGWIT;
    } else if (addr[0] == '1') {
        format = P2_LEGACY;
    } else {
        format = P2_SEGWIT;
    }

    char address[51];
    if (!get_address(format, compressed_public_key, address, sizeof(address))) {
        return false;
    }

    if (strcmp(address, addr) != 0) {
        return false;
    }

    return true;
}

static bool btc_get_printable_amount(const uint8_t *bytes,
                                     const size_t size,
                                     char *out,
                                     const size_t maxsize)
{
    if (size > 8) {
        return false;
    }

    strncpy(out, "BTC ", maxsize);

    uint8_t amount[8] = { 0 };
    memcpy(amount + (8 - size), bytes, size);
    btchip_convert_hex_amount_to_displayable(amount, out + 4);

    return true;
}

const swap_config_t btc_config = {
    btc_validate_address,
    btc_get_printable_amount,
    NULL,
};
