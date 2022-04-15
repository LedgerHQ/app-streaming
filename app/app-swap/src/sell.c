#include "swap.h"

const char *handle_init_sell(const RequestInitSell *req,
                             ResponseInitSell *response,
                             swap_ctx_t *ctx)
{
    return NULL;
}

const char *handle_sell(const RequestSell *req, ResponseSell *response, swap_ctx_t *swap_ctx)
{
    if (!verify_partner(&req->partner)) {
        return "invalid partner";
    }

    return NULL;
}
