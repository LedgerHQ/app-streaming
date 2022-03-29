#pragma once

#include <stdint.h>

#define PAGE_SIZE 256
#define PAGE_MASK ~(PAGE_SIZE-1)

#define PAGE_START(addr) ((addr) & PAGE_MASK)

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
    char name[32];
    char version[16];
    uint8_t app_hash[32];
    uint8_t pubkey_hash[32];
    uint32_t pc;
    uint32_t bss;

    struct section_s sections[NUM_SECTIONS];

    uint8_t merkle_tree_root_hash[32];
    uint32_t merkle_tree_size;
    uint8_t last_entry_init[8];
} __attribute__((packed));

void stream_init_app(uint8_t *buffer, size_t size);
void stream_run_app(void);

uint8_t *get_buffer(uint32_t addr, size_t size, bool writeable);
