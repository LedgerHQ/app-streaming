#include <stdint.h>

#include "ecall.h"
#include "ecall_hash.h"
#include "rv.h"

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
        sys_fatal(GP(RV_REG_A0));
        stop = true;
        break;
    case ECALL_XSEND:
        success = sys_xsend(GP(RV_REG_A0), cpu->regs[RV_REG_A1]);
        break;
    case ECALL_XRECV:
        success = sys_xrecv(GP(RV_REG_A0), cpu->regs[RV_REG_A1], &cpu->regs[RV_REG_A0]);
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
        sys_ux_bitmap(cpu->regs[RV_REG_A0], cpu->regs[RV_REG_A1], cpu->regs[RV_REG_A2], cpu->regs[RV_REG_A3], GP(RV_REG_A4), cpu->regs[RV_REG_A5], GP(RV_REG_A6), cpu->regs[RV_REG_A7]);
        break;
#endif
    case ECALL_WAIT_BUTTON:
        cpu->regs[RV_REG_A0] = sys_wait_button();
        break;
    case ECALL_BAGL_DRAW:
        sys_bagl_draw_with_context(GP(RV_REG_A0), GP(RV_REG_A1), cpu->regs[RV_REG_A2], cpu->regs[RV_REG_A3]);
        break;
    case ECALL_LOADING_START:
        sys_app_loading_start(GP(RV_REG_A0));
        break;
    case ECALL_LOADING_STOP:
        cpu->regs[RV_REG_A0] = sys_app_loading_stop();
        break;
    case ECALL_UX_IDLE:
        sys_ux_idle();
        break;
    case ECALL_MEMSET:
        success = sys_memset(GP(RV_REG_A0), cpu->regs[RV_REG_A1], cpu->regs[RV_REG_A2], &cpu->regs[RV_REG_A0]);
        break;
    case ECALL_MEMCPY:
        success = sys_memcpy(GP(RV_REG_A0), GP(RV_REG_A1), cpu->regs[RV_REG_A2], &cpu->regs[RV_REG_A0]);
        break;
    case ECALL_STRLEN:
        success = sys_strlen(GP(RV_REG_A0), &cpu->regs[RV_REG_A0]);
        break;
    case ECALL_STRNLEN:
        success = sys_strnlen(GP(RV_REG_A0), cpu->regs[RV_REG_A1], &cpu->regs[RV_REG_A0]);
        break;
    /*
     * XXX - Disclaimer: the following ECALLs aren't stable, probably buggy.
     *       The API is a WIP.
     */
    case ECALL_SHA256SUM:
        success = sys_sha256sum(GP(RV_REG_A0), cpu->regs[RV_REG_A1], GP(RV_REG_A2));
        break;
    case ECALL_DERIVE_NODE_BIP32:
        cpu->regs[RV_REG_A0] = sys_derive_node_bip32(cpu->regs[RV_REG_A0], GP(RV_REG_A1), cpu->regs[RV_REG_A2], GP(RV_REG_A3), GP(RV_REG_A4));
        break;
    case ECALL_CX_ECFP_GENERATE_PAIR:
        cpu->regs[RV_REG_A0] = sys_ecfp_generate_pair(cpu->regs[RV_REG_A0], GP(RV_REG_A1), GP(RV_REG_A2));
        break;
    case ECALL_CX_ECFP_GET_PUBKEY:
        cpu->regs[RV_REG_A0] = sys_ecfp_get_pubkey(cpu->regs[RV_REG_A0], GP(RV_REG_A1), GP(RV_REG_A2));
        break;
    case ECALL_CX_SHA3_256:
        success = sys_sha3_256(GP(RV_REG_A0), cpu->regs[RV_REG_A1], GP(RV_REG_A2));
        break;
    case ECALL_ECDSA_SIGN:
        cpu->regs[RV_REG_A0] = sys_ecdsa_sign(GP(RV_REG_A0), cpu->regs[RV_REG_A1], cpu->regs[RV_REG_A2], GP(RV_REG_A3), GP(RV_REG_A4), cpu->regs[RV_REG_A5]);
        break;
    case ECALL_MULT:
        sys_mult(GP(RV_REG_A0), GP(RV_REG_A1), GP(RV_REG_A2), cpu->regs[RV_REG_A3]);
        break;
    case ECALL_MULTM:
        sys_multm(GP(RV_REG_A0), GP(RV_REG_A1), GP(RV_REG_A2), GP(RV_REG_A3), cpu->regs[RV_REG_A4]);
        break;
    case ECALL_TOSTRING256:
        cpu->regs[RV_REG_A0] = sys_tostring256(GP(RV_REG_A0), cpu->regs[RV_REG_A1], GP(RV_REG_A2), cpu->regs[RV_REG_A3]);
        break;
    case ECALL_HASH_UPDATE:
        success = sys_hash_update(cpu->regs[RV_REG_A0], GP(RV_REG_A1), GP(RV_REG_A2), cpu->regs[RV_REG_A3], &cpu->regs[RV_REG_A0]);
        break;
    case ECALL_HASH_FINAL:
        cpu->regs[RV_REG_A0] = sys_hash_final(cpu->regs[RV_REG_A0], GP(RV_REG_A1), GP(RV_REG_A2));
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
