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

typedef union ctx_hash_s {
    struct ctx_sha256_s {
        size_t blen;
        uint8_t block[64];
        uint8_t acc[8 * 4];
    } sha256;

    struct ctx_sha3_s {
        size_t output_size;
        size_t block_size;
        size_t blen;
        uint8_t block[200];
        uint64_t acc[25];
    } sha3;
} __attribute__((packed)) ctx_hash_guest_t;

typedef enum cx_hash_id_e {
    HASH_ID_SHA3_256,
    HASH_ID_SHA256,
} cx_hash_id_t;
