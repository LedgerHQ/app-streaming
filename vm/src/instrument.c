/*
 * Instrumentation functions called when the app is built with
 * -finstrument-functions.
 *
 * It writes on stderr lines such as:
 *
 *    (tv_sec.tv_usec    fn_addr  call_site)
 *
 *   < 624da6ed.000401ee 40006fed 400078c7
 *   > 624da6ed.000401fa 4000991d 400078d3
 *
 * Don't forget to add `__attribute__((no_instrument_function))` to the
 * pic_internal function to avoid badly written assembly-code messing up with
 * the instrumentation functions.
 */

#ifdef WITH_INSTRUMENT

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef unsigned long time_t;
typedef unsigned long suseconds_t;

struct timeval {
    time_t tv_sec;
    suseconds_t tv_usec;
};

static __attribute__((no_instrument_function)) inline void err(char *msg)
{
    asm volatile("movs r0, #0x04\n"
                 "movs r1, %0\n"
                 "svc      0xab\n" ::"r"(msg)
                 : "r0", "r1", "memory");
}

static __attribute__((no_instrument_function)) inline void clock(struct timeval *tv)
{
    /* call the Linux 32-bit syscall gettimeofday */
    asm volatile("movs r7, #78\n"
                 "mov  r0, %0\n"
                 "svc      0\n" ::"r"(tv)
                 : "r0", "r1", "r7", "memory");
}

static char hex[16] = "0123456789abcdef";
static char buf[40];

static __attribute__((no_instrument_function)) void u32hex(uint32_t n, char *buf)
{
    for (size_t i = 0; i < 4; i++) {
        buf[i * 2] = hex[(n >> ((24 - i * 8) + 4)) & 0xf];
        buf[i * 2 + 1] = hex[(n >> (24 - i * 8)) & 0xf];
    }
}

static __attribute__((no_instrument_function)) void output(bool enter,
                                                           void *this_fn,
                                                           void *call_site)
{
    buf[0] = enter ? '>' : '<';
    buf[1] = ' ';
    buf[10] = '.';
    buf[19] = ' ';
    buf[28] = ' ';
    buf[37] = '\n';
    buf[38] = '\x00';

    struct timeval tv;
    clock(&tv);

    u32hex(tv.tv_sec, &buf[2]);
    u32hex(tv.tv_usec, &buf[11]);

    u32hex((uint32_t)this_fn, &buf[20]);
    u32hex((uint32_t)call_site, &buf[29]);

    err(buf);
}

void __attribute__((no_instrument_function))
__cyg_profile_func_enter(void *this_fn, void *call_site)
{
    output(true, this_fn, call_site);
}

void __attribute__((no_instrument_function)) __cyg_profile_func_exit(void *this_fn, void *call_site)
{
    output(false, this_fn, call_site);
}

#endif
