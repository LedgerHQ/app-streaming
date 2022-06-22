#pragma once

#include <stdint.h>

#include "bagl.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#endif

enum ux_action_e {
    UX_ACTION_NONE,
    UX_ACTION_VALIDATE,
    UX_ACTION_REJECT,
};

struct ux_item_s {
    const bagl_icon_details_t *icon;
    uint8_t *line1;
    uint8_t *line2;
    enum ux_action_e action;
};

void display_item(struct ux_item_s *item, bool first, bool last);
bool ux_validate(struct ux_item_s *items, size_t count);
