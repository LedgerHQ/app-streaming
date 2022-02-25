#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pb_decode.h>
#include <pb_encode.h>
#include <pb_common.h>

#include "message.pb.h"

#include "app-ethereum.h"
#include "ui.h"
#include "sdk.h"

#define MAX_REQ_SIZE 1024

static char *handle_get_version(RequestGetVersion *req, ResponseGetVersion *response)
{
    strncpy(response->version, "version 1.33.7", sizeof(response->version));
    return NULL;
}

static void set_error(Response *response, const char *msg)
{
    response->which_message_oneof = Response_error_tag;

    ResponseError *error = &response->message_oneof.error;
    strncpy(error->error_msg, msg, sizeof(error->error_msg));
    error->error_msg[sizeof(error->error_msg) - 1] = '\x00';
}

static void handle_req(Request *req, Response *response)
{
    const char *error;

    switch (req->which_message_oneof) {
    case Request_get_version_tag:
        response->which_message_oneof = Response_get_version_tag;
        error = handle_get_version(&req->message_oneof.get_version, &response->message_oneof.get_version);
        break;
    case Request_get_pubkey_tag:
        response->which_message_oneof = Response_get_pubkey_tag;
        error = handle_get_pubkey(&req->message_oneof.get_pubkey, &response->message_oneof.get_pubkey);
        break;
    default:
        error = "invalid request tag";
        break;
    }

    if (error != NULL) {
        set_error(response, error);
    }
}

int main(void)
{
    ux_idle();

    while (1) {
        Response response;
        Request req;

        uint8_t buf[MAX_REQ_SIZE];
        size_t req_len = xrecv(buf, sizeof(buf));

        app_loading_start();

        pb_istream_t istream = pb_istream_from_buffer(buf, req_len);
        if (pb_decode(&istream, Request_fields, &req)) {
            handle_req(&req, &response);
        } else {
            set_error(&response, PB_GET_ERROR(&istream));
        }

        pb_ostream_t ostream = pb_ostream_from_buffer(buf, sizeof(buf));
        if (!pb_encode(&ostream, Response_fields, &response)) {
            fatal("failed to encode response");
        }

        app_loading_stop();

        xsend(buf, ostream.bytes_written);

        ux_idle();
    }

    return 0;
}
