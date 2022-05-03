#include <stdbool.h>

#include "cx.h"
#include "no_throw.h"
#include "syscalls.h"

#ifndef SYSCALL_os_perso_derive_node_bip32_ID
#define SYSCALL_os_perso_derive_node_bip32_ID SYSCALL_os_perso_derive_node_bip32_ID_IN
#endif

__attribute__((naked)) bool svc_call_no_throw(unsigned int id __attribute__((unused)),
                                              ptrdiff_t *parameters __attribute__((unused)))
{
    asm volatile(
                 "svc  1\n"
                 "mov  r0, r1\n"
                 "rsbs r1, r0, #0\n"
                 "adcs r0, r1\n"
                 "bx   lr\n"
                 ::: "r0", "r1"
                 );
}

bool os_perso_derive_node_bip32_nt(cx_curve_t curve, const uint32_t *path, size_t pathLength, uint8_t *privateKey, uint8_t *chain)
{
    ptrdiff_t parameters[5];

    parameters[0] = (ptrdiff_t)curve;
    parameters[1] = (ptrdiff_t)path;
    parameters[2] = (ptrdiff_t)pathLength;
    parameters[3] = (ptrdiff_t)privateKey;
    parameters[4] = (ptrdiff_t)chain;

    return svc_call_no_throw(SYSCALL_os_perso_derive_node_bip32_ID, parameters);
}
