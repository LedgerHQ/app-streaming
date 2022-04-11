#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "page.h"

enum section_e {
    SECTION_CODE,
    SECTION_STACK,
    SECTION_DATA,
    NUM_SECTIONS,
};

struct section_s {
    uint32_t start;
    uint32_t end;
} __attribute__((packed));

struct manifest_s {
    uint32_t manifest_version; /* not used for now */
    char name[32];
    char version[16];
    uint8_t app_hash[32];
    uint32_t pc;
    uint32_t bss;

    struct section_s sections[NUM_SECTIONS];

    uint8_t merkle_tree_root_hash[32];
    uint32_t merkle_tree_size;
    uint8_t last_entry_init[8];
} __attribute__((packed));

void stream_init_app(uint8_t *buffer, size_t size);
void stream_run_app(void);
bool mem_read(const uint32_t addr, const size_t size, uint32_t *value);
void mem_write(uint32_t addr, size_t size, uint32_t value);
