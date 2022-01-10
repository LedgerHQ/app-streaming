#pragma once

#include "os_task.h"

static inline void err(char *msg)
{
    asm volatile (
     "movs r0, #0x04\n"
     "movs r1, %0\n"
     "svc      0xab\n"
     :: "r"(msg) : "r0", "r1"
    );
}

static inline void fatal(char *msg)
{
    err(msg);
    os_sched_exit(7);
}
