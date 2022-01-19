#include "ux_flow_engine.h"

#include "config.h"
#include "glyphs.h"
#include "ux.h"

ux_state_t G_ux;

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
