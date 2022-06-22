#include <stdbool.h>
#include <stdlib.h>

#include "ecall.h"
#include "sdk.h"
#include "ux/ux.h"

size_t xrecv(uint8_t *buffer, size_t size)
{
    return ecall_xrecv(buffer, size);
}

void xrecvall(uint8_t *buffer, size_t size)
{
    while (size > 0) {
        size_t n = xrecv(buffer, size);
        buffer += n;
        size -= n;
    }
}

void bagl_draw_with_context(const bagl_component_t *component,
                            const void *context,
                            unsigned short context_length,
                            unsigned char context_encoding)
{
    packed_bagl_component_t packed_component;

    packed_component.type = component->type;
    packed_component.userid = component->userid;
    packed_component.x = component->x;
    packed_component.y = component->y;
    packed_component.width = component->width;
    packed_component.height = component->height;
    packed_component.stroke = component->stroke;
    packed_component.radius = component->radius;
    packed_component.fill = component->fill;
    packed_component.fgcolor = component->fgcolor;
    packed_component.bgcolor = component->bgcolor;
    packed_component.font_id = component->font_id;
    packed_component.icon_id = component->icon_id;

    ecall_bagl_draw_with_context(&packed_component, context, context_length, context_encoding);
}

void bagl_hal_draw_bitmap_within_rect(int x,
                                      int y,
                                      unsigned int width,
                                      unsigned int height,
                                      unsigned int color_count,
                                      const unsigned int *colors,
                                      unsigned int bit_per_pixel,
                                      const unsigned char *bitmap,
                                      unsigned int bitmap_length_bits)
{
    /* don't pass color_count since it's always 2 */
    ecall_bagl_hal_draw_bitmap_within_rect(x, y, width, height, colors, bit_per_pixel, bitmap,
                                           bitmap_length_bits);
}
