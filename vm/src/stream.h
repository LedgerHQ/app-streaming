#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "memory.h"

#include "cx.h"

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
    uint8_t last_entry_digest_init[CX_SHA256_SIZE];
} __attribute__((packed));

bool stream_init_app(const uint8_t *buffer, const size_t size);
void stream_run_app(void);
bool stream_commit_page(struct page_s *page, bool insert);
bool stream_request_page(struct page_s *page, const uint32_t addr, const bool read_only);
bool mem_read(const uint32_t addr, const size_t size, uint32_t *value);
bool mem_write(const uint32_t addr, const size_t size, const uint32_t value);
