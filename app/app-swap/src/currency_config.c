#include <string.h>

#include "currency_config.h"

#include "btc/btc.h"
#include "eth/eth.h"

const swap_config_t *get_config(const char *currency)
{
    if (strcmp(currency, "BTC") == 0) {
        return &btc_config;
    }

    if (strcmp(currency, "ETH") == 0) {
        return &eth_config;
    }

    return NULL;
}
