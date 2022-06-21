#pragma once

#define BAGL_NOFILL  0
#define BAGL_FILL    1
#define BAGL_OUTLINE 2
#define BAGL_NOICON  3

typedef enum bagl_components_type_e_ {
    BAGL_NONE = 0, // for fast memset clearing all components
    BAGL_BUTTON = 1,
    // BAGL_TEXTBOX, // NOT NECESSARY
    // BAGL_SENSEZONE, // sense only
    BAGL_LABEL,
    BAGL_RECTANGLE,
    BAGL_LINE,
    BAGL_ICON,
    BAGL_CIRCLE,
    BAGL_LABELINE, // label for which y coordinate is the baseline of the text,
                   // does not support vertical alignment
    BAGL_FLAG_TOUCHABLE = 0x80,
} bagl_components_type_e;

typedef struct bagl_component_s {
    bagl_components_type_e type;
    unsigned char userid;
    short x; // allow for out of screen rendering
    short y; // allow for out of screen rendering
    unsigned short width;
    unsigned short height;
    unsigned char stroke;
    unsigned char radius;
    unsigned char fill;
    unsigned int fgcolor;
    unsigned int bgcolor;
    unsigned short font_id;
    unsigned char icon_id;
} bagl_component_t;

typedef struct bagl_icon_details_s {
    unsigned int width;
    unsigned int height;
    unsigned int bpp;
    const unsigned int *colors;
    const unsigned char *bitmap;
} bagl_icon_details_t;

typedef struct bagl_element_e {
    bagl_component_t component;
    const char *text;
} bagl_element_t;
