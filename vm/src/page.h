#pragma once

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
