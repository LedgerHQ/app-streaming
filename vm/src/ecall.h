#pragma once

#include <stdbool.h>
#include <stdint.h>

struct rv_cpu;

typedef struct guest_pointer_s {
    uint32_t addr;
} guest_pointer_t;

void copy_guest_buffer(guest_pointer_t p_src, void *buf, size_t size);
void copy_host_buffer(guest_pointer_t p_dst, void *buf, size_t size);
void sys_exit(uint32_t code);

bool ecall(struct rv_cpu *cpu);
bool ecall_bolos(struct rv_cpu *cpu, uint32_t nr);

#define GP(reg) ((const guest_pointer_t){ .addr = cpu->regs[reg] })
