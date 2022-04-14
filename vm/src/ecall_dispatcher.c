#include <stdint.h>

#include "ecall.h"
#include "ecall_hash.h"
#include "rv_cpu.h"

#include "sdk/api/ecall-nr.h"

/*
 * Return true if the ecall either exit() or unsupported, false otherwise.
 */
bool ecall(struct rv_cpu *cpu)
{
    uint32_t nr = cpu->regs[RV_REG_T0];
    bool stop = false;
    bool success = true;

    switch (nr) {
    case ECALL_FATAL:
        success = sys_fatal(GP(RV_REG_A0));
        stop = true;
        break;
    case ECALL_XSEND:
        success = sys_xsend(GP(RV_REG_A0), cpu->regs[RV_REG_A1]);
        break;
    case ECALL_XRECV:
        success = sys_xrecv(ERET(RV_REG_A0), GP(RV_REG_A0), cpu->regs[RV_REG_A1]);
        break;
    case ECALL_EXIT:
        sys_exit(cpu->regs[RV_REG_A0]);
        stop = true;
        break;
#ifdef TARGET_NANOX
    case ECALL_UX_RECTANGLE:
        sys_ux_rectangle(cpu->regs[RV_REG_A0], cpu->regs[RV_REG_A1], cpu->regs[RV_REG_A2], cpu->regs[RV_REG_A3], cpu->regs[RV_REG_A4]);
        break;
    case ECALL_SCREEN_UPDATE:
        sys_screen_update();
        break;
    case ECALL_BAGL_DRAW_BITMAP:
        success = sys_ux_bitmap(cpu->regs[RV_REG_A0], cpu->regs[RV_REG_A1], cpu->regs[RV_REG_A2], cpu->regs[RV_REG_A3], GP(RV_REG_A4), cpu->regs[RV_REG_A5], GP(RV_REG_A6), cpu->regs[RV_REG_A7]);
        break;
#endif
    case ECALL_WAIT_BUTTON:
        cpu->regs[RV_REG_A0] = sys_wait_button();
        break;
    case ECALL_BAGL_DRAW:
        sys_bagl_draw_with_context(GP(RV_REG_A0), GP(RV_REG_A1), cpu->regs[RV_REG_A2], cpu->regs[RV_REG_A3]);
        break;
    case ECALL_LOADING_START:
        success = sys_app_loading_start(GP(RV_REG_A0));
        break;
    case ECALL_LOADING_STOP:
        cpu->regs[RV_REG_A0] = sys_app_loading_stop();
        break;
    case ECALL_UX_IDLE:
        sys_ux_idle();
        break;
    case ECALL_MEMSET:
        success = sys_memset(ERET(RV_REG_A0), GP(RV_REG_A0), cpu->regs[RV_REG_A1], cpu->regs[RV_REG_A2]);
        break;
    case ECALL_MEMCPY:
        success = sys_memcpy(ERET(RV_REG_A0), GP(RV_REG_A0), GP(RV_REG_A1), cpu->regs[RV_REG_A2]);
        break;
    case ECALL_STRLEN:
        success = sys_strlen(ERET(RV_REG_A0), GP(RV_REG_A0));
        break;
    case ECALL_STRNLEN:
        success = sys_strnlen(ERET(RV_REG_A0), GP(RV_REG_A0), cpu->regs[RV_REG_A1]);
        break;
    /*
     * XXX - Disclaimer: the following ECALLs aren't stable, probably buggy.
     *       The API is a WIP.
     */
    case ECALL_DERIVE_NODE_BIP32:
        success = sys_derive_node_bip32(ERET(RV_REG_A0), cpu->regs[RV_REG_A0], GP(RV_REG_A1), cpu->regs[RV_REG_A2], GP(RV_REG_A3), GP(RV_REG_A4));
        break;
    case ECALL_CX_ECFP_GENERATE_PAIR:
        success = _sys_cx_ecfp_generate_pair(ERET(RV_REG_A0), cpu->regs[RV_REG_A0], GP(RV_REG_A1), GP(RV_REG_A2), cpu->regs[RV_REG_A3]);
        break;
    case ECALL_ECDSA_SIGN:
        success = sys_ecdsa_sign(ERET(RV_REG_A0), GP(RV_REG_A0), cpu->regs[RV_REG_A1], cpu->regs[RV_REG_A2], GP(RV_REG_A3), GP(RV_REG_A4), cpu->regs[RV_REG_A5]);
        break;
    case ECALL_MULT:
        success = sys_mult(ERET(RV_REG_A0), GP(RV_REG_A0), GP(RV_REG_A1), GP(RV_REG_A2), cpu->regs[RV_REG_A3]);
        break;
    case ECALL_MULTM:
        success = sys_multm(ERET(RV_REG_A0), GP(RV_REG_A0), GP(RV_REG_A1), GP(RV_REG_A2), GP(RV_REG_A3), cpu->regs[RV_REG_A4]);
        break;
    case ECALL_TOSTRING256:
        success = sys_tostring256(ERET(RV_REG_A0), GP(RV_REG_A0), cpu->regs[RV_REG_A1], GP(RV_REG_A2), cpu->regs[RV_REG_A3]);
        break;
    case ECALL_HASH_UPDATE:
        success = sys_hash_update(ERET(RV_REG_A0), cpu->regs[RV_REG_A0], GP(RV_REG_A1), GP(RV_REG_A2), cpu->regs[RV_REG_A3]);
        break;
    case ECALL_HASH_FINAL:
        success = sys_hash_final(ERET(RV_REG_A0), cpu->regs[RV_REG_A0], GP(RV_REG_A1), GP(RV_REG_A2));
        break;
    default:
        sys_exit(0xdeaddead);
        stop = true;
        break;
    }

    if (!success) {
        stop = true;
    }

    return stop;
}
