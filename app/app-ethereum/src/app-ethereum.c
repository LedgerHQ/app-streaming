#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app-ethereum.h"
#include "ui.h"
#include "sdk.h"

#define MAX_REQ_SIZE 1024

static ResponseGetVersion *handle_get_version(RequestGetVersion *req)
{
    ALLOC_RESPONSE(ResponseGetVersion, RESPONSE_GET_VERSION__INIT);

    response->version = strdup("version 1.33.7");

    return response;
}

static ResponseError *handle_error(void)
{
    ALLOC_RESPONSE(ResponseError, RESPONSE_ERROR__INIT);

    response->error_msg = strdup("invalid message");

    return response;
}

static void handle_req(Request *req, Response *response)
{
    switch (req->message_oneof_case) {
        case REQUEST__MESSAGE_ONEOF_GET_VERSION:
            response->message_oneof_case = RESPONSE__MESSAGE_ONEOF_GET_VERSION;
            response->get_version = handle_get_version(req->get_version);
            break;

        case REQUEST__MESSAGE_ONEOF_GET_PUBKEY:
            response->message_oneof_case = RESPONSE__MESSAGE_ONEOF_GET_PUBKEY;
            response->get_pubkey = handle_get_pubkey(req->get_pubkey);
            if (response->get_pubkey == NULL) {
                response->message_oneof_case = RESPONSE__MESSAGE_ONEOF_ERROR;
                response->error = handle_error();
            }
            break;

        case REQUEST__MESSAGE_ONEOF_SIGN_TX:
            response->message_oneof_case = RESPONSE__MESSAGE_ONEOF_SIGN_TX;
            response->sign_tx = handle_sign_tx(req->sign_tx);
            break;

        default:
            response->message_oneof_case = RESPONSE__MESSAGE_ONEOF_ERROR;
            response->error = handle_error();
            break;
    }
}

static void free_response(Response *response)
{
    switch (response->message_oneof_case) {
        case RESPONSE__MESSAGE_ONEOF_GET_VERSION:
            free(response->get_version->version);
            free(response->get_version);
            break;

        case RESPONSE__MESSAGE_ONEOF_GET_PUBKEY:
            if (response->get_pubkey->has_pubkey) {
                free(response->get_pubkey->pubkey.data);
            }
            if (response->get_pubkey->has_address) {
                free(response->get_pubkey->address.data);
            }
            if (response->get_pubkey->has_chain_code) {
                free(response->get_pubkey->chain_code.data);
            }
            free(response->get_pubkey);
            break;

        case RESPONSE__MESSAGE_ONEOF_SIGN_TX:
            free(response->sign_tx->signature.data);
            free(response->sign_tx);
            break;

        case RESPONSE__MESSAGE_ONEOF_ERROR:
            free(response->error->error_msg);
            free(response->error);
            break;

        case RESPONSE__MESSAGE_ONEOF__NOT_SET:
        default:
            break;
    }
}

int main(void)
{
    ui_menu_main();

    while (1) {
        /* receive buffer */
        uint8_t buf[MAX_REQ_SIZE];
        size_t req_len = xrecv(buf, MAX_REQ_SIZE);

        app_loading_start();

        /* unpack it */
        Request *req = request__unpack(NULL, req_len, buf);
        if (req == NULL) {
            fatal("invalid request");
        }

        /* parse and process the request */
        Response response = RESPONSE__INIT;
        handle_req(req, &response);
        request__free_unpacked(req, NULL);

        if (!protobuf_c_message_check(&response.base)) {
            fatal("internal error during encoding");
        }

        /* alloc buffer for the response */
        size_t len = response__get_packed_size(&response);
        uint8_t *buf2 = xmalloc(len);

        /* pack the response */
        size_t size = response__pack(&response, buf2);
        free_response(&response);

        app_loading_stop();

        /* send it */
        xsend(buf2, size);
        free(buf2);

        ui_menu_main();
    }

    return 0;
}
