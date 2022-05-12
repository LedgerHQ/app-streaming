#pragma once

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

void init_memory_sections(const struct section_s *sections);
void init_caches(void);
void init_memory_addresses(uint32_t bss, uint32_t sp);

struct page_s *get_page(uint32_t addr, enum page_prot_e page_prot);
struct page_s *get_code_page(const uint32_t addr);
