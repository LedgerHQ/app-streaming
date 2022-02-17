#include "glyphs.h"
        
unsigned int const C_icon_left_colors[] = {
  0x00000000,
  0x00ffffff,
};

unsigned char const C_icon_left_bitmap[] = {
  0x48, 0x12, 0x42, 0x08,
};

const bagl_icon_details_t C_icon_left = { GLYPH_icon_left_WIDTH, GLYPH_icon_left_HEIGHT, 1, C_icon_left_colors, C_icon_left_bitmap };

unsigned int const C_icon_right_colors[] = {
  0x00000000,
  0x00ffffff,
};

unsigned char const C_icon_right_bitmap[] = {
  0x21, 0x84, 0x24, 0x01,
};

const bagl_icon_details_t C_icon_right = { GLYPH_icon_right_WIDTH, GLYPH_icon_right_HEIGHT, 1, C_icon_right_colors, C_icon_right_bitmap };

const bagl_icon_details_t C_boilerplate_logo = { GLYPH_boilerplate_logo_WIDTH, GLYPH_boilerplate_logo_HEIGHT, 1, C_boilerplate_logo_colors, C_boilerplate_logo_bitmap };
        
unsigned int const C_boilerplate_logo_colors[] = {
  0x00000000,
  0x00ffffff,
};

unsigned char const C_boilerplate_logo_bitmap[] = {
  0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe, 0x7f, 0xfe, 0x3f, 0xfc, 0x3f, 0xfc, 0x01, 0x80, 0x07, 0xe0,
  0x0f, 0xf0, 0x1f, 0xf8, 0x0f, 0xf0, 0x8f, 0xf1, 0xc7, 0xe3, 0xe7, 0xe7, 0xff, 0xff, 0xff, 0xff,
};

unsigned int const C_icon_dashboard_colors[] = {
  0x00000000,
  0x00ffffff,
};

unsigned char const C_icon_dashboard_bitmap[] = {
  0xe0, 0x01, 0xfe, 0xc1, 0xff, 0x38, 0x70, 0x06, 0xd8, 0x79, 0x7e, 0x9e, 0x9f, 0xe7, 0xe7, 0xb9,
  0x01, 0xe6, 0xc0, 0xf1, 0x3f, 0xf8, 0x07, 0x78, 0x00,
};

const bagl_icon_details_t C_icon_dashboard = { GLYPH_icon_dashboard_WIDTH, GLYPH_icon_dashboard_HEIGHT, 1, C_icon_dashboard_colors, C_icon_dashboard_bitmap };

unsigned int const C_icon_dashboard_x_colors[] = {
  0x00000000,
  0x00ffffff,
};

unsigned char const C_icon_dashboard_x_bitmap[] = {
  0x00, 0x00, 0x00, 0x00, 0x0c, 0x80, 0x07, 0xf0, 0x03, 0xfe, 0xc1, 0xff, 0xf0, 0x3f, 0xf0, 0x03,
  0xcc, 0x00, 0x33, 0xc0, 0x0c, 0x00, 0x00, 0x00, 0x00,
};

const bagl_icon_details_t C_icon_dashboard_x = { GLYPH_icon_dashboard_x_WIDTH, GLYPH_icon_dashboard_x_HEIGHT, 1, C_icon_dashboard_x_colors, C_icon_dashboard_x_bitmap };
