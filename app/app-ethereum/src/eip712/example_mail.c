// https://docs.rs/eip-712/latest/eip_712/
// https://github.com/spruceid/ssi/blob/main/src/eip712.rs
// https://github.com/ethereum/EIPs/blob/master/assets/eip-712/Example.js

#include <stdio.h>
#include <string.h>

#include <jsmn.h>

#include "app-ethereum.h"
#include "sdk.h"
#include "ux/glyphs.h"
#include "ux/ux.h"

#include "ui.h"

#include "eip712.h"

static member_data_t from_members[] = {
    { .type = TYPE_STRING },  // name
    { .type = TYPE_ADDRESS }, // wallet
};

static member_data_t to_members[] = {
    { .type = TYPE_STRING },  // name
    { .type = TYPE_ADDRESS }, // wallet
};

// Person(string name,address wallet)
static hash_struct_t from = {
    .type_hash = "\xb9\xd8\xc7\x8a\xcf\x9b\x98\x73\x11\xde\x6c\x7b\x45\xbb\x6a\x9c\x8e\x1b\xf3\x61"
                 "\xfa\x7f\xd3\x46\x7a\x21\x63\xf9\x94\xc7\x95\x00",
    .count = ARRAY_SIZE(from_members),
    .members = from_members,
};

// Person(string name,address wallet)
static hash_struct_t to = {
    .type_hash = "\xb9\xd8\xc7\x8a\xcf\x9b\x98\x73\x11\xde\x6c\x7b\x45\xbb\x6a\x9c\x8e\x1b\xf3\x61"
                 "\xfa\x7f\xd3\x46\x7a\x21\x63\xf9\x94\xc7\x95\x00",
    .count = ARRAY_SIZE(to_members),
    .members = to_members,
};

static member_data_t mail_members[] = {
    { .type = TYPE_STRUCT, .hstruct = &from },
    { .type = TYPE_STRUCT, .hstruct = &to },
    { .type = TYPE_STRING }, // contents
};

// Mail(Person from,Person to,string contents)Person(string name,address wallet)
static hash_struct_t mail = {
    .type_hash = "\xa0\xce\xde\xb2\xdc\x28\x0b\xa3\x9b\x85\x75\x46\xd7\x4f\x55\x49\xc3\xa1\xd7\xbd"
                 "\xc2\xdd\x96\xbf\x88\x1f\x76\x10\x8e\x23\xda\xc2",
    .count = ARRAY_SIZE(mail_members),
    .members = mail_members,
};

static json_field_t mail_fields[] = {
    { .type = FIELD_KEY, .key = "from" },     { .type = FIELD_OTHER, .other = true },
    { .type = FIELD_KEY, .key = "name" },     { .type = FIELD_VALUE, .value = &from_members[0] },
    { .type = FIELD_KEY, .key = "wallet" },   { .type = FIELD_VALUE, .value = &from_members[1] },
    { .type = FIELD_KEY, .key = "to" },       { .type = FIELD_OTHER, .other = true },
    { .type = FIELD_KEY, .key = "name" },     { .type = FIELD_VALUE, .value = &to_members[0] },
    { .type = FIELD_KEY, .key = "wallet" },   { .type = FIELD_VALUE, .value = &to_members[1] },
    { .type = FIELD_KEY, .key = "contents" }, { .type = FIELD_VALUE, .value = &mail_members[2] },
};

static char g_from_name[80];
static char g_from_address[80];
static char g_to_name[80];
static char g_to_address[80];
static char g_content[80];

UX_STEP_NOCB(ux_example_mail_1_step, pnn, { &C_icon_eye, "Review", "mail" });
UX_STEP_NOCB(ux_example_mail_2_step, nn, { "From name:", g_from_name });
UX_STEP_NOCB(ux_example_mail_3_step, nn, { "From address:", g_from_address });
UX_STEP_NOCB(ux_example_mail_4_step, nn, { "To name:", g_to_name });
UX_STEP_NOCB(ux_example_mail_5_step, nn, { "To address:", g_to_address });
UX_STEP_NOCB(ux_example_mail_6_step, nn, { "Content:", g_content });
UX_STEP_CB(ux_example_mail_7_step,
           pbb,
           eip712_validated = 1,
           { &C_icon_validate_14, "Accept", "and sign" });
UX_STEP_CB(ux_example_mail_8_step, pb, eip712_validated = -1, { &C_icon_crossmark, "Reject" });

UX_FLOW(ux_example_mail_flow,
        &ux_example_mail_1_step,
        &ux_example_mail_2_step,
        &ux_example_mail_3_step,
        &ux_example_mail_4_step,
        &ux_example_mail_5_step,
        &ux_example_mail_6_step,
        &ux_example_mail_7_step,
        &ux_example_mail_8_step);

static bool validate_ui(void)
{
    app_loading_start("Preparing UI...");

    copy_string(g_from_name, sizeof(g_from_name), &from_members[0].string);
    copy_string(g_to_name, sizeof(g_to_name), &to_members[0].string);
    copy_address(g_from_address, sizeof(g_from_address), &from_members[1].address);
    copy_address(g_to_address, sizeof(g_to_address), &to_members[1].address);
    copy_string(g_content, sizeof(g_content), &mail_members[2].string);

    return ui_eip712((const ux_flow_step_t *const *)&ux_example_mail_flow);
}

const hash_struct_t *eip712_example_mail(const char *json_string, jsmntok_t *t, int token_count)
{
    if (token_count != ARRAY_SIZE(mail_fields)) {
        return NULL;
    }

    if (!extract_fields(mail_fields, json_string, t, token_count)) {
        return NULL;
    }

    if (!validate_ui()) {
        return NULL;
    }

    return &mail;
}
