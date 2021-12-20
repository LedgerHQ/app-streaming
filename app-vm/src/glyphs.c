#include "glyphs.h"
unsigned int const C_boilerplate_logo_colors[] = {
  0x00000000,
  0x00ffffff,
};

unsigned char const C_boilerplate_logo_bitmap[] = {
  0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe, 0x7f, 0xfe, 0x3f, 0xfc, 0x3f, 0xfc, 0x01, 0x80, 0x07, 0xe0,
  0x0f, 0xf0, 0x1f, 0xf8, 0x0f, 0xf0, 0x8f, 0xf1, 0xc7, 0xe3, 0xe7, 0xe7, 0xff, 0xff, 0xff, 0xff,
};
#ifdef HAVE_BAGL
        #include "bagl.h"
        const bagl_icon_details_t C_boilerplate_logo = { GLYPH_boilerplate_logo_WIDTH, GLYPH_boilerplate_logo_HEIGHT, 1, C_boilerplate_logo_colors, C_boilerplate_logo_bitmap };
        #endif // HAVE_BAGL
#include "glyphs.h"
unsigned int const C_icon_down_colors[] = {
  0x00000000,
  0x00ffffff,
};

unsigned char const C_icon_down_bitmap[] = {
  0x41, 0x11, 0x05, 0x01,
};
#ifdef HAVE_BAGL
        #include "bagl.h"
        const bagl_icon_details_t C_icon_down = { GLYPH_icon_down_WIDTH, GLYPH_icon_down_HEIGHT, 1, C_icon_down_colors, C_icon_down_bitmap };
        #endif // HAVE_BAGL
#include "glyphs.h"
unsigned int const C_icon_left_colors[] = {
  0x00000000,
  0x00ffffff,
};

unsigned char const C_icon_left_bitmap[] = {
  0x48, 0x12, 0x42, 0x08,
};
#ifdef HAVE_BAGL
        #include "bagl.h"
        const bagl_icon_details_t C_icon_left = { GLYPH_icon_left_WIDTH, GLYPH_icon_left_HEIGHT, 1, C_icon_left_colors, C_icon_left_bitmap };
        #endif // HAVE_BAGL
#include "glyphs.h"
unsigned int const C_icon_right_colors[] = {
  0x00000000,
  0x00ffffff,
};

unsigned char const C_icon_right_bitmap[] = {
  0x21, 0x84, 0x24, 0x01,
};
#ifdef HAVE_BAGL
        #include "bagl.h"
        const bagl_icon_details_t C_icon_right = { GLYPH_icon_right_WIDTH, GLYPH_icon_right_HEIGHT, 1, C_icon_right_colors, C_icon_right_bitmap };
        #endif // HAVE_BAGL
#include "glyphs.h"
unsigned int const C_icon_up_colors[] = {
  0x00000000,
  0x00ffffff,
};

unsigned char const C_icon_up_bitmap[] = {
  0x08, 0x8a, 0x28, 0x08,
};
#ifdef HAVE_BAGL
        #include "bagl.h"
        const bagl_icon_details_t C_icon_up = { GLYPH_icon_up_WIDTH, GLYPH_icon_up_HEIGHT, 1, C_icon_up_colors, C_icon_up_bitmap };
        #endif // HAVE_BAGL
