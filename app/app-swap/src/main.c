#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pb_common.h>
#include <pb_decode.h>
#include <pb_encode.h>

#include "message.pb.h"

#include "sdk.h"
#include "swap.h"

#define MAX_REQ_SIZE 4096

static const char *handle_get_version(const RequestGetVersion *req, ResponseGetVersion *response)
{
    strncpy(response->version, "version 1.33.7", sizeof(response->version));
    return NULL;
}

static void set_error(Response *response, const char *msg)
{
    response->which_response = Response_error_tag;

    ResponseError *error = &response->response.error;
    strncpy(error->error_msg, msg, sizeof(error->error_msg));
    error->error_msg[sizeof(error->error_msg) - 1] = '\x00';
}

static void reset_ctx(swap_ctx_t *ctx)
{
    memset(ctx, 0, sizeof(*ctx));
}

static void handle_req(const Request *req, Response *response, swap_ctx_t *ctx)
{
    const char *error;

    switch (req->which_request) {
    case Request_get_version_tag:
        response->which_response = Response_get_version_tag;
        error = handle_get_version(&req->request.get_version,
                                   &response->response.get_version);
        break;
    case Request_init_swap_tag:
        reset_ctx(ctx);
        response->which_response = Response_init_swap_tag;
        error = handle_init_swap(&req->request.init_swap, &response->response.init_swap,
                                 ctx);
        break;
    case Request_init_sell_tag:
        reset_ctx(ctx);
        response->which_response = Response_init_sell_tag;
        error = handle_init_sell(&req->request.init_sell, &response->response.init_sell,
                                 ctx);
        break;
    case Request_swap_tag:
        response->which_response = Response_swap_tag;
        error = handle_swap(&req->request.swap, &response->response.swap, ctx);
        reset_ctx(ctx);
        break;
    case Request_sell_tag:
        response->which_response = Response_sell_tag;
        error = handle_sell(&req->request.sell, &response->response.sell, ctx);
        reset_ctx(ctx);
        break;
    default:
        error = "invalid request tag";
        break;
    }

    if (error != NULL) {
        reset_ctx(ctx);
        set_error(response, error);
    }
}

int main(void)
{
    ux_idle();

    swap_ctx_t ctx;
    reset_ctx(&ctx);

    while (1) {
        Response response;
        Request req;

        uint8_t buf[MAX_REQ_SIZE];
        const size_t req_len = xrecv(buf, sizeof(buf));

        app_loading_start("Decoding request...");

        pb_istream_t istream = pb_istream_from_buffer(buf, req_len);
        if (pb_decode(&istream, Request_fields, &req)) {
            handle_req(&req, &response, &ctx);
        } else {
            set_error(&response, PB_GET_ERROR(&istream));
        }

        pb_ostream_t ostream = pb_ostream_from_buffer(buf, sizeof(buf));
        if (!pb_encode(&ostream, Response_fields, &response)) {
            fatal("failed to encode response");
        }

        app_loading_stop();
        ux_idle();

        xsend(buf, ostream.bytes_written);
    }

    return 0;
}
