#include <stdint.h>
#include <stdlib.h>

#include "rlp.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    struct tx_s tx;

    tx.to = NULL;
    tx.data = NULL;

    rlp_decode_list(data, size, &tx);

    free(tx.to);
    free(tx.data);

    return 0;
}
