#include <stdbool.h>
#include <stdlib.h>

#include "ecall.h"
#include "sdk.h"
#include "ux/ux.h"

int puts(const char *str)
{
    asm (
         "li t0, 1\n"
         "add a0, %0, 0\n"
         "ecall\n"
         :: "r"(str) : "t0", "a0"
         );

    return 0;
}

void xsend(const uint8_t *buffer, size_t size)
{
    register uint32_t a0 asm ("a0") = (uint32_t)buffer;
    register uint32_t a1 asm ("a1") = (uint32_t)size;

    asm (
         "li t0, %0\n"
         "ecall\n"
         :: "i"(ECALL_XSEND), "r"(a0), "r"(a1) : "t0", "memory"
         );
}

#define BUTTON_MAGIC 0xdeadbe00

static void button_helper(uint8_t magic)
{
    /*unsigned int button_mask;
    unsigned int button_same_mask_counter = 0; // ignored

    button_mask = magic | BUTTON_EVT_RELEASED;

    if (G_ux.stack[0].button_push_callback != NULL) {
        G_ux.stack[0].button_push_callback(button_mask, button_same_mask_counter);
        }*/
    switch(magic) {
    case 1:
      ux_flow_prev();
      break;
    case 2:
      ux_flow_next();
      break;
    case 3:
      ux_flow_validate();
      break;
  }
}

size_t xrecv_helper(uint8_t *buffer, size_t size)
{
    register uint32_t a0 asm ("a0") = (uint32_t)buffer;
    register uint32_t a1 asm ("a1") = (uint32_t)size;
    size_t ret;

    asm volatile (
         "li t0, %1\n"
         "ecall\n"
         "add %0, a0, 0\n"
         : "=r"(ret) : "i"(ECALL_XRECV), "r"(a0), "r"(a1) : "t0", "memory"
         );

    return ret;
}

size_t xrecv(uint8_t *buffer, size_t size)
{
    bool is_magic_button;
    size_t ret;

    do {
        ret = xrecv_helper(buffer, size);
        is_magic_button = ((ret & BUTTON_MAGIC) == BUTTON_MAGIC);
        if (is_magic_button) {
            button_helper(ret & 0xff);
        }
    } while (is_magic_button);

    return ret;
}

void xrecvall(uint8_t *buffer, size_t size)
{
    while (size > 0) {
        size_t n = xrecv(buffer, size);
        buffer += n;
        size -= n;
    }
}

void sha256sum(const uint8_t *buffer, size_t size, uint8_t *digest)
{
    register uint32_t a0 asm ("a0") = (uint32_t)buffer;
    register uint32_t a1 asm ("a1") = (uint32_t)size;
    register uint32_t a2 asm ("a2") = (uint32_t)digest;

    asm (
         "li t0, %0\n"
         "ecall\n"
         :: "i"(ECALL_SHA256SUM), "r"(a0), "r"(a1), "r"(a2) : "t0", "memory"
         );

}

void screen_update(void)
{
    asm (
         "li t0, %0\n"
         "ecall\n"
         :: "i"(ECALL_SCREEN_UPDATE) : "t0"
        );
}

void bagl_hal_draw_rect(unsigned int color, int x, int y, unsigned int width, unsigned int height)
{
    register uint32_t a0 asm ("a0") = (uint32_t)color;
    register uint32_t a1 asm ("a1") = (uint32_t)x;
    register uint32_t a2 asm ("a2") = (uint32_t)y;
    register uint32_t a3 asm ("a3") = (uint32_t)width;
    register uint32_t a4 asm ("a4") = (uint32_t)height;

    asm (
         "li t0, %0\n"
         "ecall\n"
         :: "i"(ECALL_UX_RECTANGLE), "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4) : "t0"
        );
}

void bagl_hal_draw_bitmap_within_rect(int x, int y, unsigned int width, unsigned int height, unsigned int color_count, const unsigned int * colors, unsigned int bit_per_pixel, const unsigned char * bitmap, unsigned int bitmap_length_bits)
{
    register uint32_t a0 asm ("a0") = (uint32_t)x;
    register uint32_t a1 asm ("a1") = (uint32_t)y;
    register uint32_t a2 asm ("a2") = (uint32_t)width;
    register uint32_t a3 asm ("a3") = (uint32_t)height;
    register uint32_t a4 asm ("a4") = (uint32_t)colors;
    register uint32_t a5 asm ("a5") = (uint32_t)bit_per_pixel;
    register uint32_t a6 asm ("a6") = (uint32_t)bitmap;
    register uint32_t a7 asm ("a7") = (uint32_t)bitmap_length_bits;

    asm volatile (
         "li t0, %0\n"
         "ecall\n"
         :: "i"(ECALL_BAGL_DRAW_BITMAP), "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7) : "t0"
                  );
}

int wait_button(void)
{
    int button;

    asm volatile (
         "li t0, %1\n"
         "ecall\n"
         "add %0, a0, 0\n"
         : "=r"(button) : "i"(ECALL_WAIT_BUTTON) : "a0", "t0"
                  );

    return button;
}

typedef struct {
  unsigned char type;
  unsigned char userid;
  short x;
  short y;
  unsigned short width;
  unsigned short height;
  unsigned char stroke;
  unsigned char radius;
  unsigned char fill;
  unsigned int fgcolor;
  unsigned int bgcolor;
  unsigned short font_id;
  unsigned char icon_id;
} __attribute__((packed)) packed_bagl_component_t;

void bagl_draw_with_context(const bagl_component_t *component, const void *context, unsigned short context_length, unsigned char context_encoding)
{
    volatile packed_bagl_component_t packed_component;

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

    register uint32_t a0 asm ("a0") = (uint32_t)&packed_component;
    register uint32_t a1 asm ("a1") = (uint32_t)context;
    register uint32_t a2 asm ("a2") = (uint32_t)context_length;
    register uint32_t a3 asm ("a3") = (uint32_t)context_encoding;

    asm volatile (
         "li t0, %0\n"
         "ecall\n"
         :: "i"(ECALL_BAGL_DRAW), "r"(a0), "r"(a1), "r"(a2), "r"(a3) : "t0"
                  );
}
