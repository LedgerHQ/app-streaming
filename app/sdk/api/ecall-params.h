#pragma once

#include <stdint.h>

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
