#include <stdio.h>
#include <string.h>

#include <jsmn.h>

#include "app-ethereum.h"
#include "sdk.h"
#include "ux/glyphs.h"
#include "ux/ux.h"

#include "ui.h"

#include "eip712.h"

static member_data_t order_members[] = {
    { .type = TYPE_ADDRESS },                           // exchange
    { .type = TYPE_ADDRESS },                           // maker
    { .type = TYPE_ADDRESS },                           // taker
    { .type = TYPE_UINT_N, .uint_n = { .count = 32 } }, // makerRelayerFee
    { .type = TYPE_UINT_N, .uint_n = { .count = 32 } }, // takerRelayerFee
    { .type = TYPE_UINT_N, .uint_n = { .count = 32 } }, // makerProtocolFee
    { .type = TYPE_UINT_N, .uint_n = { .count = 32 } }, // takerProtocolFee
    { .type = TYPE_ADDRESS },                           // feeRecipient
    { .type = TYPE_UINT_N, .uint_n = { .count = 1 } },  // feeMethod
    { .type = TYPE_UINT_N, .uint_n = { .count = 1 } },  // side
    { .type = TYPE_UINT_N, .uint_n = { .count = 1 } },  // saleKind
    { .type = TYPE_ADDRESS },                           // target
    { .type = TYPE_UINT_N, .uint_n = { .count = 1 } },  // howToCall
    { .type = TYPE_BYTES },                             // calldata
    { .type = TYPE_BYTES },                             // replacementPattern
    { .type = TYPE_ADDRESS },                           // staticTarget
    { .type = TYPE_BYTES },                             // staticExtradata
    { .type = TYPE_ADDRESS },                           // paymentToken
    { .type = TYPE_UINT_N, .uint_n = { .count = 32 } }, // basePrice
    { .type = TYPE_UINT_N, .uint_n = { .count = 32 } }, // extra
    { .type = TYPE_UINT_N, .uint_n = { .count = 32 } }, // listingTime
    { .type = TYPE_UINT_N, .uint_n = { .count = 32 } }, // expirationTime
    { .type = TYPE_UINT_N, .uint_n = { .count = 32 } }, // salt
    { .type = TYPE_UINT_N, .uint_n = { .count = 32 } }, // nonce
};

static hash_struct_t order = {
    .type_hash = "\xdb\xa0\x8a\x88\xa7\x48\xf3\x56\xe8\xfa\xf8\x57\x84\x88\x34\x3e\xab\x21\xb1\x74"
                 "\x17\x28\x77\x9c\x9d\xcf\xdc\x78\x2b\xc8\x00\xf8",
    .count = ARRAY_SIZE(order_members),
    .members = order_members,
};

static json_field_t order_fields[] = {
    { .type = FIELD_KEY, .key = "exchange" },
    { .type = FIELD_VALUE, .value = &order_members[0] },
    { .type = FIELD_KEY, .key = "maker" },
    { .type = FIELD_VALUE, .value = &order_members[1] },
    { .type = FIELD_KEY, .key = "taker" },
    { .type = FIELD_VALUE, .value = &order_members[2] },
    { .type = FIELD_KEY, .key = "makerRelayerFee" },
    { .type = FIELD_VALUE, .value = &order_members[3] },
    { .type = FIELD_KEY, .key = "takerRelayerFee" },
    { .type = FIELD_VALUE, .value = &order_members[4] },
    { .type = FIELD_KEY, .key = "makerProtocolFee" },
    { .type = FIELD_VALUE, .value = &order_members[5] },
    { .type = FIELD_KEY, .key = "takerProtocolFee" },
    { .type = FIELD_VALUE, .value = &order_members[6] },
    { .type = FIELD_KEY, .key = "feeRecipient" },
    { .type = FIELD_VALUE, .value = &order_members[7] },
    { .type = FIELD_KEY, .key = "feeMethod" },
    { .type = FIELD_VALUE, .value = &order_members[8] },
    { .type = FIELD_KEY, .key = "side" },
    { .type = FIELD_VALUE, .value = &order_members[9] },
    { .type = FIELD_KEY, .key = "saleKind" },
    { .type = FIELD_VALUE, .value = &order_members[10] },
    { .type = FIELD_KEY, .key = "target" },
    { .type = FIELD_VALUE, .value = &order_members[11] },
    { .type = FIELD_KEY, .key = "howToCall" },
    { .type = FIELD_VALUE, .value = &order_members[12] },
    { .type = FIELD_KEY, .key = "calldata" },
    { .type = FIELD_VALUE, .value = &order_members[13] },
    { .type = FIELD_KEY, .key = "replacementPattern" },
    { .type = FIELD_VALUE, .value = &order_members[14] },
    { .type = FIELD_KEY, .key = "staticTarget" },
    { .type = FIELD_VALUE, .value = &order_members[15] },
    { .type = FIELD_KEY, .key = "staticExtradata" },
    { .type = FIELD_VALUE, .value = &order_members[16] },
    { .type = FIELD_KEY, .key = "paymentToken" },
    { .type = FIELD_VALUE, .value = &order_members[17] },
    { .type = FIELD_KEY, .key = "basePrice" },
    { .type = FIELD_VALUE, .value = &order_members[18] },
    { .type = FIELD_KEY, .key = "extra" },
    { .type = FIELD_VALUE, .value = &order_members[19] },
    { .type = FIELD_KEY, .key = "listingTime" },
    { .type = FIELD_VALUE, .value = &order_members[20] },
    { .type = FIELD_KEY, .key = "expirationTime" },
    { .type = FIELD_VALUE, .value = &order_members[21] },
    { .type = FIELD_KEY, .key = "salt" },
    { .type = FIELD_VALUE, .value = &order_members[22] },
    { .type = FIELD_KEY, .key = "nonce" },
    { .type = FIELD_VALUE, .value = &order_members[23] },

};

static char g_maker[80];
static char g_taker[80];
static char g_fees[80];
static char g_base_price[80];

UX_STEP_NOCB(ux_opensea_1_step, pnn, { &C_icon_eye, "Review", "OpenSea message" });
UX_STEP_NOCB(ux_opensea_2_step, nn, { "Maker:", g_maker });
UX_STEP_NOCB(ux_opensea_3_step, nn, { "Taker:", g_taker });
UX_STEP_NOCB(ux_opensea_4_step, nn, { "Fees:", g_fees });
UX_STEP_NOCB(ux_opensea_5_step, nn, { "Base Price:", g_base_price });
UX_STEP_CB(ux_opensea_6_step,
           pbb,
           eip712_validated = 1,
           { &C_icon_validate_14, "Accept", "and sign" });
UX_STEP_CB(ux_opensea_7_step, pb, eip712_validated = -1, { &C_icon_crossmark, "Reject" });

UX_FLOW(ux_opensea_flow,
        &ux_opensea_1_step,
        &ux_opensea_2_step,
        &ux_opensea_3_step,
        &ux_opensea_4_step,
        &ux_opensea_5_step,
        &ux_opensea_6_step,
        &ux_opensea_7_step);

static bool validate_ui(void)
{
    uint64_t chain_id = 1;

    app_loading_start("Preparing UI...");

    copy_address(g_maker, sizeof(g_maker), &order_members[1].address);
    copy_address(g_taker, sizeof(g_taker), &order_members[2].address);
    copy_address(g_fees, sizeof(g_fees), &order_members[7].address);
    copy_amount(chain_id, g_base_price, sizeof(g_base_price), &order_members[18].uint_n);

    return ui_eip712((const ux_flow_step_t *const *)&ux_opensea_flow);
}

const hash_struct_t *eip712_opensea(const char *json_string, jsmntok_t *t, int token_count)
{
    if (token_count != ARRAY_SIZE(order_fields)) {
        return false;
    }

    if (!extract_fields(order_fields, json_string, t, token_count)) {
        return NULL;
    }

    if (!validate_ui()) {
        return NULL;
    }

    return &order;
}
