#pragma once

#include <stdint.h>

// PAGE_SIZE can be defined by libc headers, for test targets
#undef PAGE_SIZE

#define PAGE_SIZE 256
#define PAGE_MASK ~(PAGE_SIZE - 1)

#define PAGE_START(addr)      ((addr)&PAGE_MASK)

/* returns the maximum size of a guest buffer within a page */
#define BUFFER_MAX_SIZE(addr) (PAGE_SIZE - (addr - PAGE_START(addr)))

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

/* returns the minimum size of a guest buffer within a page */
#define BUFFER_MIN_SIZE(addr, size) MIN(BUFFER_MAX_SIZE(addr), size)

enum page_prot_e {
    PAGE_PROT_RO,
    PAGE_PROT_RW,
};

struct page_s {
    uint32_t addr;
    uint8_t data[PAGE_SIZE];
    uint32_t iv;
};
