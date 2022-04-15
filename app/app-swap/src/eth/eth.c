#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crypto.h"
#include "eth.h"
#include "sdk.h"
#include "tx.h"

#define CHAIN_CODE_SIZE 32
#define WEI_TO_ETHER    18

static const char *derive_pubkey(const uint32_t *path,
                                 size_t path_count,
                                 cx_ecfp_public_key_t *pubkey,
                                 uint8_t *chain_code)
{
    const char *error = NULL;

    uint8_t privkey_data[32];
    if (!derive_node_bip32(CX_CURVE_256K1, path, path_count, privkey_data, chain_code)) {
        error = "path derivation failed";
        goto end;
    }

    cx_ecfp_private_key_t privkey;
    if (!ecfp_generate_keypair(CX_CURVE_256K1, privkey_data, sizeof(privkey_data), pubkey,
                               &privkey)) {
        error = "failed to generate keypair";
        goto end;
    }

end:
    explicit_bzero(privkey_data, sizeof(privkey_data));
    explicit_bzero(&privkey, sizeof(privkey));

    return error;
}

static bool eth_validate_address(const char *addr, const uint32_t *path, const size_t path_count)
{
    cx_ecfp_public_key_t pubkey;
    uint8_t chain_code[CHAIN_CODE_SIZE];
    const char *error = derive_pubkey(path, path_count, &pubkey, chain_code);
    if (error != NULL) {
        return false;
    }

    size_t offset;
    char address[43];
    if (strncmp(addr, "0x", 2) == 0) {
        address[0] = '0';
        address[1] = 'x';
        offset = 2;
    } else {
        offset = 0;
    }
    uint64_t chainId = 1; /* XXX */
    getEthAddressStringFromKey(&pubkey, address + offset, chainId);

    if (strcmp(address, addr) != 0) {
        return false;
    }

    return true;
}

static bool eth_get_printable_amount(const uint8_t *bytes,
                                     const size_t size,
                                     char *out,
                                     const size_t maxsize)
{
    if (size > 32) {
        return false;
    }

    /*
    // If the amount is a fee, its value is nominated in ETH even if we're doing an ERC20 swap
    if (params->is_fee) {
        uint8_t ticker_len = strnlen(config->coinName, sizeof(config->coinName));
        memcpy(ticker, config->coinName, ticker_len);
        ticker[ticker_len] = ' ';
        ticker[ticker_len + 1] = '\0';
        decimals = WEI_TO_ETHER;
    } else {
        // If the amount is *not* a fee, decimals and ticker are built from the given config
        if (!parse_swap_config(params->coin_configuration,
                               params->coin_configuration_length,
                               ticker,
                               &decimals)) {
            PRINTF("Error while parsing config\n");
            return 0;
        }
        }*/

    /* XXX */
    const char *ticker = "ETH ";
    uint8_t decimals = WEI_TO_ETHER;
    amountToString(bytes, size, decimals, ticker, out, maxsize);

    return true;
}

const swap_config_t eth_config = {
    eth_validate_address,
    eth_get_printable_amount,
    eth_create_tx,
};
