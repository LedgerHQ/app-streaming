#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include "ux_flow_engine.h"

#include "config.h"
#include "glyphs.h"
#include "ux.h"

ux_state_t G_ux;

int
__clzsi2 (int val)
{
  int i = 32;
  int j = 16;
  int temp;

  for (; j; j >>= 1)
    {
        temp = val >> j;
      if (temp)
	{
	  if (j == 1)
	    {
	      return (i - 2);
	    }
	  else
	    {
	      i -= j;
	      val = temp;
	    }
	}
    }
  return (i - val);
}

unsigned int __udivsi3(unsigned int n, unsigned int d)
{
    const unsigned n_uword_bits = sizeof(unsigned int) * CHAR_BIT;
    unsigned int q;
    unsigned int r;
    unsigned sr;
    /* special cases */
    if (d == 0)
        return 0; /* ?! */
    if (n == 0)
        return 0;
    sr = __builtin_clz(d) - __builtin_clz(n);
    /* 0 <= sr <= n_uword_bits - 1 or sr large */
    if (sr > n_uword_bits - 1)  /* d > r */
        return 0;
    if (sr == n_uword_bits - 1)  /* d == 1 */
        return n;
    ++sr;
    /* 1 <= sr <= n_uword_bits - 1 */
    /* Not a special case */
    q = n << (n_uword_bits - sr);
    r = n >> sr;
    unsigned int carry = 0;
    for (; sr > 0; --sr)
    {
        /* r:q = ((r:q)  << 1) | carry */
        r = (r << 1) | (q >> (n_uword_bits - 1));
        q = (q << 1) | carry;
        /* carry = 0;
         * if (r.all >= d.all)
         * {
         *      r.all -= d.all;
         *      carry = 1;
         * }
         */
        const int s = (int)(d - r - 1) >> (n_uword_bits - 1);
        carry = s & 1;
        r -= d & s;
    }
    q = (q << 1) | carry;
    return q;
}

void screen_update(void)
{
    asm (
         "li t0, 7\n"
         "ecall\n"
         ::: "t0"
         );
}

void bagl_hal_draw_rect(unsigned int color, int x, int y, unsigned int width, unsigned int height)
{
    asm (
         "li t0, 6\n"
         "ecall\n"
         );
}

//void bagl_hal_draw_bitmap_within_rect_(int x, int y, unsigned int width, unsigned int height/*, unsigned int color_count*/, const unsigned int * colors, unsigned int bit_per_pixel, const unsigned char * bitmap, unsigned int bitmap_length_bits) {
//}

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
         "li t0, 8\n"
         "ecall\n"
         :: "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7)
                  );
}

static void os_sched_exit(int x)
{
    //exit(x);
}

void io_seproxyhal_display_icon(const bagl_component_t* icon_component, const bagl_icon_details_t* icon_details) {
  bagl_component_t icon_component_mod;

  if (icon_details == NULL || icon_details->bitmap == NULL) {
      return;
  }

  // ensure not being out of bounds in the icon component agianst the declared icon real size
  memcpy(&icon_component_mod, icon_component, sizeof(bagl_component_t));
  icon_component_mod.width = icon_details->width;
  icon_component_mod.height = icon_details->height;
  icon_component = &icon_component_mod;

#ifdef HAVE_SE_SCREEN
  bagl_draw_glyph(&icon_component_mod, icon_details);
#else
  if (io_seproxyhal_spi_is_status_sent()) {
      return;
  }
  // color index size
  unsigned int h = (1<<(icon_details->bpp))*sizeof(unsigned int);
  // bitmap size
  unsigned int w = ((icon_component->width*icon_component->height*icon_details->bpp)/8)+((icon_component->width*icon_component->height*icon_details->bpp)%8?1:0);
  unsigned short length = sizeof(bagl_component_t)
      +1 /* bpp */
      +h /* color index */
      +w; /* image bitmap size */
  G_io_seproxyhal_spi_buffer[0] = SEPROXYHAL_TAG_SCREEN_DISPLAY_STATUS;
  G_io_seproxyhal_spi_buffer[1] = length>>8;
  G_io_seproxyhal_spi_buffer[2] = length;
  io_seproxyhal_spi_send(G_io_seproxyhal_spi_buffer, 3);
  io_seproxyhal_spi_send((unsigned char*)icon_component, sizeof(bagl_component_t));
  G_io_seproxyhal_spi_buffer[0] = icon_details->bpp;
  io_seproxyhal_spi_send(G_io_seproxyhal_spi_buffer, 1);
  io_seproxyhal_spi_send((unsigned char*)PIC(icon_details->colors), h);
  io_seproxyhal_spi_send((unsigned char*)PIC(icon_details->bitmap), w);
#endif
}

static void display_icon(const bagl_component_t* icon_component, const bagl_icon_details_t* icon_det)
{
    io_seproxyhal_display_icon(icon_component, icon_det);
}

static void display_text(const bagl_component_t *component, const char *txt)
{
#ifdef HAVE_SE_SCREEN
    bagl_draw_with_context(component, txt, strlen(txt), BAGL_ENCODING_LATIN1);
#else
    if (io_seproxyhal_spi_is_status_sent()) {
        return;
    }
    unsigned short length = sizeof(bagl_component_t)+strlen(txt);
    G_io_seproxyhal_spi_buffer[0] = SEPROXYHAL_TAG_SCREEN_DISPLAY_STATUS;
    G_io_seproxyhal_spi_buffer[1] = length>>8;
    G_io_seproxyhal_spi_buffer[2] = length;
    io_seproxyhal_spi_send(G_io_seproxyhal_spi_buffer, 3);
    io_seproxyhal_spi_send((uint8_t *)component, sizeof(bagl_component_t));
    io_seproxyhal_spi_send((uint8_t *)txt, length-sizeof(bagl_component_t));
#endif
}

static void display_component(const bagl_component_t *component)
{
#ifdef HAVE_SE_SCREEN
    bagl_draw_with_context(component, NULL, 0, 0);
#else
    if (io_seproxyhal_spi_is_status_sent()) {
        return;
    }
    unsigned short length = sizeof(bagl_component_t);
    G_io_seproxyhal_spi_buffer[0] = SEPROXYHAL_TAG_SCREEN_DISPLAY_STATUS;
    G_io_seproxyhal_spi_buffer[1] = length>>8;
    G_io_seproxyhal_spi_buffer[2] = length;
    io_seproxyhal_spi_send(G_io_seproxyhal_spi_buffer, 3);
    io_seproxyhal_spi_send((uint8_t *)component, sizeof(bagl_component_t));
#endif
}

void io_seproxyhal_display(const bagl_element_t *element)
{
    const char* txt = (const char*)element->text;

    // process automagically address from rom and from ram
    unsigned int type = (element->component.type & ~(BAGL_FLAG_TOUCHABLE));

    if (type != BAGL_NONE) {
        if (txt != NULL) {
            // consider an icon details descriptor is pointed by the context
            if (type == BAGL_ICON && element->component.icon_id == 0) {
                // SECURITY: due to this wild cast, the code MUST be executed on the application side instead of in
                //           the syscall sides to avoid buffer overflows and a real hard way of checking buffer
                //           belonging in the syscall dispatch
                display_icon((bagl_component_t*)&element->component, (bagl_icon_details_t*)txt);
            }
            else {
                display_text(&element->component, txt);
            }
        }
        else {
            display_component(&element->component);
        }
    }
}

UX_STEP_NOCB(ux_menu_ready_step, pn, {&C_boilerplate_logo, "Fuckin' RISC app"});
UX_STEP_VALID(ux_menu_exit_step, pb, os_sched_exit(-1), {&C_icon_dashboard_x, "Quit"});

UX_FLOW(ux_menu_main_flow,
        &ux_menu_ready_step,
        &ux_menu_exit_step,
        FLOW_LOOP);

void ui_menu_main(void)
{
    memset(&G_ux, 0, sizeof(G_ux));

    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }

    ux_flow_init(0, ux_menu_main_flow, NULL);
}
