#pragma once

#include <stdbool.h>
#include <stdint.h>

struct rv_cpu;

void copy_guest_buffer(uint32_t addr, void *buf, size_t size);
void copy_host_buffer(uint32_t addr, void *buf, size_t size);
void sys_exit(uint32_t code);

bool ecall(struct rv_cpu *cpu);
bool ecall_bolos(struct rv_cpu *cpu, uint32_t nr);
