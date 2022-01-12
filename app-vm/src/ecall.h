#pragma once

#include <stdbool.h>

struct rv_cpu;

bool ecall(struct rv_cpu *cpu);
