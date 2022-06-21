#include <string.h>

#include "ecall.h"
#include "fonts.h"
#include "glyphs.h"
#include "sdk.h"
#include "ux.h"

static const bagl_element_t ux_erase = { { BAGL_RECTANGLE, 0x00, 0, 0, 128, 64, 0, 0, BAGL_FILL,
                                           0x000000, 0xFFFFFF, 0, 0 },
                                         NULL };
static const bagl_element_t ux_left = { { BAGL_ICON, 0x01, 2, 28, 4, 7, 0, 0, 0, 0xFFFFFF, 0x000000,
                                          0, 0 },
                                        (const char *)&C_icon_left };
static const bagl_element_t ux_right = { { BAGL_ICON, 0x02, 122, 28, 4, 7, 0, 0, 0, 0xFFFFFF,
                                           0x000000, 0, 0 },
                                         (const char *)&C_icon_right };

static bagl_element_t ux_layout_pbb_elements[] = {
    { { BAGL_ICON, 0x0F, 57, 10, 14, 14, 0, 0, 0, 0xFFFFFF, 0x000000,
        BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0 },
      NULL },
    { { BAGL_LABELINE, 0x10, 6, 37, 116, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
        BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 0 },
      NULL },
    { { BAGL_LABELINE, 0x11, 6, 51, 116, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
        BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 0 },
      NULL },
};

static bagl_element_t ux_layout_pn_elements[] = {
    { { BAGL_ICON, 0x10, 57, 17, 14, 14, 0, 0, 0, 0xFFFFFF, 0x000000,
        BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0 },
      NULL },
    { { BAGL_LABELINE, 0x11, 0, 44, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
        BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 0 },
      NULL },
};

static bagl_element_t ux_layout_bb_elements[] = {
    { { BAGL_LABELINE, 0x10, 6, 29, 116, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
        BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0 },
      NULL },
    { { BAGL_LABELINE, 0x11, 6, 43, 116, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
        BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0 },
      NULL },
};

static void bagl_draw_glyph(const bagl_component_t *component,
                            const bagl_icon_details_t *icon_details)
{
    bagl_hal_draw_bitmap_within_rect(
        component->x, component->y, icon_details->width, icon_details->height,
        1 << (icon_details->bpp), icon_details->colors, icon_details->bpp, icon_details->bitmap,
        icon_details->bpp * (icon_details->width * icon_details->height));
}

static void display_icon(const bagl_component_t *component, const bagl_icon_details_t *details)
{
    bagl_component_t icon_component_mod;

    memcpy(&icon_component_mod, component, sizeof(bagl_component_t));

    icon_component_mod.width = details->width;
    icon_component_mod.height = details->height;

    bagl_draw_glyph(&icon_component_mod, details);
}

static void display_text(const bagl_component_t *component, const char *txt)
{
    bagl_draw_with_context(component, txt, strlen(txt), BAGL_ENCODING_LATIN1);
}

static void display_rectangle(const bagl_component_t *component)
{
    bagl_draw_with_context(component, NULL, 0, 0);
}

static void display_element(const bagl_element_t *element)
{
    switch (element->component.type) {
    case BAGL_RECTANGLE:
        display_rectangle(&element->component);
        break;
    case BAGL_ICON:
        display_icon(&element->component, (const bagl_icon_details_t *)element->text);
        break;
    case BAGL_LABELINE:
        display_text(&element->component, (const char *)element->text);
        break;
    default:
        break;
    }
}

static void display_elements(const bagl_element_t *elements, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        display_element(&elements[i]);
    }
}

void display_item(struct ux_item_s *item, bool first, bool last)
{
    display_element(&ux_erase);

    if (!first) {
        display_element(&ux_left);
    }

    if (!last) {
        display_element(&ux_right);
    }

    if (item->icon != NULL) {
        if (item->line2 != NULL) {
            ux_layout_pbb_elements[0].text = (const char *)item->icon;
            ux_layout_pbb_elements[1].text = item->line1;
            ux_layout_pbb_elements[2].text = item->line2;
            display_elements(ux_layout_pbb_elements, ARRAY_SIZE(ux_layout_pbb_elements));
        } else {
            ux_layout_pn_elements[0].text = (const char *)item->icon;
            ux_layout_pn_elements[1].text = item->line1;
            display_elements(ux_layout_pn_elements, ARRAY_SIZE(ux_layout_pn_elements));
        }
    } else {
        ux_layout_bb_elements[0].text = item->line1;
        ux_layout_bb_elements[1].text = item->line2;
        display_elements(ux_layout_bb_elements, ARRAY_SIZE(ux_layout_bb_elements));
    }
}
