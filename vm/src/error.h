#pragma once

#ifndef NATIVE
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "os_id.h"
#include "os_task.h"

static bool initialized;
static bool on_speculos;

static bool running_on_speculos(void)
{
    uint8_t buffer[32] = { 0 };
    os_version(buffer, sizeof(buffer));
    return strcmp((char *)buffer, "Speculos") == 0;
}

static inline void err(char *msg)
{
    if (!initialized) {
        on_speculos = running_on_speculos();
        initialized = true;
    }

    if (!on_speculos) {
        return;
    }

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
#else
#include <stdio.h>
#include <stdlib.h>

static inline void err(char *msg)
{
    fprintf(stderr, "%s", msg);
}

static inline void fatal(char *msg)
{
    err(msg);
    exit(7);
}
#endif
