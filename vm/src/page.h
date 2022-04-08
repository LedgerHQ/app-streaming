#pragma once

#define PAGE_SIZE 256
#define PAGE_MASK ~(PAGE_SIZE - 1)

#define PAGE_START(addr)      ((addr)&PAGE_MASK)

/* returns the maximum size of a guest buffer within a page */
#define BUFFER_MAX_SIZE(addr) (PAGE_SIZE - (addr - PAGE_START(addr)))
