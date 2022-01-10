#pragma once

#include <stdint.h>

#define PAGE_SIZE 256
#define PAGE_MASK ~(PAGE_SIZE-1)

#define PAGE_START(addr) ((addr) & PAGE_MASK)

void stream_init_app(uint8_t *buffer);
void stream_run_app(void);

uint8_t *get_buffer(uint32_t addr, size_t size, bool writeable);
