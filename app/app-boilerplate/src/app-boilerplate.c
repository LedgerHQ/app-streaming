#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app-boilerplate.h"
#include "sdk.h"
#include "ux/glyphs.h"
#include "ux/ux.h"

#define MAX_MSG_SIZE 1024

UX_STEP_VALID(ux_menu_ready_step, pn, exit(0), {&C_boilerplate_logo, "Boilerplate app"});
UX_FLOW(ux_menu_main_flow,
        &ux_menu_ready_step);

static ResponseGetVersion *handle_get_version(MessageGetVersion *msg)
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

static void handle_msg(Message *msg, Response *response)
{
    switch (msg->message_oneof_case) {
        case MESSAGE__MESSAGE_ONEOF_GET_VERSION:
            response->message_oneof_case = RESPONSE__MESSAGE_ONEOF_GET_VERSION;
            response->get_version = handle_get_version(msg->get_version);
            break;

        case MESSAGE__MESSAGE_ONEOF_GET_PUBKEY:
            response->message_oneof_case = RESPONSE__MESSAGE_ONEOF_GET_PUBKEY;
            break;

        case MESSAGE__MESSAGE_ONEOF_SIGN_TX:
            response->message_oneof_case = RESPONSE__MESSAGE_ONEOF_SIGN_TX;
            response->sign_tx = handle_sign_tx(msg->sign_tx);
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

static void ui_menu_main(void)
{
    memset(&G_ux, 0, sizeof(G_ux));
    ux_stack_push();
    ux_flow_init(0, ux_menu_main_flow, NULL);
}

int main(void)
{
    ui_menu_main();

    while (1) {
        /* receive buffer */
        uint8_t buf[MAX_MSG_SIZE];
        size_t msg_len = xrecv(buf, MAX_MSG_SIZE);

        /* unpack it */
        Message *msg = message__unpack(NULL, msg_len, buf);
        if (msg == NULL) {
            exit(1);
        }

        /* parse and process the request */
        Response response = RESPONSE__INIT;
        handle_msg(msg, &response);
        message__free_unpacked(msg, NULL);

        /* alloc buffer for the response */
        size_t len = response__get_packed_size(&response);
        uint8_t *buf2 = malloc(len);
        if (buf2 == NULL) {
            exit(2);
        }

        /* pack the response */
        size_t size = response__pack(&response, buf2);
        free_response(&response);

        /* send it */
        xsend(buf2, size);
        free(buf2);

        ui_menu_main();
    }

    return 0;
}
