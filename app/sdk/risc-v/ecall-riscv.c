#include "api/ecall-nr.h"
#include "ecall-common.h"
#include "ecall.h"

/* clang-format off */

#define ECALL0v(_name, _id)                                             \
    void _name(void)                                                    \
    {                                                                   \
        asm (                                                           \
             "li t0, %0\n"                                              \
             "ecall\n"                                                  \
             :: "i"(_id) : "t0"                                         \
        );                                                              \
    }

#define ECALL0(_name, _id, _ret_type)                                   \
    _ret_type _name(void)                                               \
    {                                                                   \
        _ret_type ret;                                                  \
        asm (                                                           \
             "li t0, %1\n"                                              \
             "ecall\n"                                                  \
             "add %0, a0, 0\n"                                          \
             : "=r"(ret) : "i"(_id) : "t0"                              \
        );                                                              \
        return ret;                                                     \
    }

#define ECALL1v(_name, _id, _type0, _arg0)               \
    void _name(_type0 _arg0)                              \
    {                                                                   \
        register uint32_t a0 asm ("a0") = (uint32_t)_arg0;              \
        asm (                                                           \
             "li t0, %0\n"                                              \
             "ecall\n"                                                  \
             :: "i"(_id), "r"(a0) : "t0", "memory"             \
        );                                                              \
    }

#define ECALL1(_name, _id, _ret_type, _type0, _arg0)                    \
    _ret_type _name(_type0 _arg0)                                       \
    {                                                                   \
        register uint32_t a0 asm ("a0") = (uint32_t)_arg0;              \
        _ret_type ret;                                                  \
        asm (                                                           \
             "li t0, %1\n"                                              \
             "ecall\n"                                                  \
             "add %0, a0, 0\n"                                          \
             : "=r"(ret) : "i"(_id), "r"(a0) : "t0", "memory"           \
        );                                                              \
        return ret;                                                     \
    }

#define ECALL2v(_name, _id, _type0, _arg0, _type1, _arg1)               \
    void _name(_type0 _arg0, _type1 _arg1)                              \
    {                                                                   \
        register uint32_t a0 asm ("a0") = (uint32_t)_arg0;              \
        register uint32_t a1 asm ("a1") = (uint32_t)_arg1;              \
        asm (                                                           \
             "li t0, %0\n"                                              \
             "ecall\n"                                                  \
             :: "i"(_id), "r"(a0), "r"(a1) : "t0", "memory"             \
        );                                                              \
    }

#define ECALL2(_name, _id, _ret_type, _type0, _arg0, _type1, _arg1)     \
    _ret_type _name(_type0 _arg0, _type1 _arg1)                         \
    {                                                                   \
        register uint32_t a0 asm ("a0") = (uint32_t)_arg0;              \
        register uint32_t a1 asm ("a1") = (uint32_t)_arg1;              \
        _ret_type ret;                                                  \
        asm (                                                           \
             "li t0, %1\n"                                              \
             "ecall\n"                                                  \
             "add %0, a0, 0\n"                                          \
             : "=r"(ret) : "i"(_id), "r"(a0), "r"(a1) : "t0", "memory"  \
        );                                                              \
        return ret;                                                     \
    }

#define ECALL3v(_name, _id, _type0, _arg0, _type1, _arg1, _type2, _arg2) \
    void _name(_type0 _arg0, _type1 _arg1, _type2 _arg2)                \
    {                                                                   \
        register uint32_t a0 asm ("a0") = (uint32_t)_arg0;              \
        register uint32_t a1 asm ("a1") = (uint32_t)_arg1;              \
        register uint32_t a2 asm ("a2") = (uint32_t)_arg2;              \
        asm (                                                           \
             "li t0, %0\n"                                              \
             "ecall\n"                                                  \
             :: "i"(_id), "r"(a0), "r"(a1), "r"(a2) : "t0", "memory"    \
        );                                                              \
    }

#define ECALL3(_name, _id, _ret_type, _type0, _arg0, _type1, _arg1, _type2, _arg2) \
    _ret_type _name(_type0 _arg0, _type1 _arg1, _type2 _arg2)           \
    {                                                                   \
        register uint32_t a0 asm ("a0") = (uint32_t)_arg0;              \
        register uint32_t a1 asm ("a1") = (uint32_t)_arg1;              \
        register uint32_t a2 asm ("a2") = (uint32_t)_arg2;              \
        _ret_type ret;                                                  \
        asm (                                                           \
             "li t0, %1\n"                                              \
             "ecall\n"                                                  \
             : "=r"(ret) : "i"(_id), "r"(a0), "r"(a1), "r"(a2) : "t0", "memory" \
        );                                                              \
        return ret;                                                     \
    }

#define ECALL4(_name, _id, _ret_type, _type0, _arg0, _type1, _arg1, _type2, _arg2, _type3, _arg3) \
    _ret_type _name(_type0 _arg0, _type1 _arg1, _type2 _arg2, _type3 _arg3) \
    {                                                                   \
        register uint32_t a0 asm ("a0") = (uint32_t)_arg0;              \
        register uint32_t a1 asm ("a1") = (uint32_t)_arg1;              \
        register uint32_t a2 asm ("a2") = (uint32_t)_arg2;              \
        register uint32_t a3 asm ("a3") = (uint32_t)_arg3;              \
        _ret_type ret;                                                  \
        asm (                                                           \
             "li t0, %1\n"                                              \
             "ecall\n"                                                  \
             : "=r"(ret) : "i"(_id), "r"(a0), "r"(a1), "r"(a2), "r"(a3) : "t0", "memory" \
        );                                                              \
        return ret;                                                     \
    }

#define ECALL4v(_name, _id, _type0, _arg0, _type1, _arg1, _type2, _arg2, _type3, _arg3) \
    void _name(_type0 _arg0, _type1 _arg1, _type2 _arg2, _type3 _arg3)  \
    {                                                                   \
        register uint32_t a0 asm ("a0") = (uint32_t)_arg0;              \
        register uint32_t a1 asm ("a1") = (uint32_t)_arg1;              \
        register uint32_t a2 asm ("a2") = (uint32_t)_arg2;              \
        register uint32_t a3 asm ("a3") = (uint32_t)_arg3;              \
        asm (                                                           \
             "li t0, %0\n"                                              \
             "ecall\n"                                                  \
             :: "i"(_id), "r"(a0), "r"(a1), "r"(a2), "r"(a3) : "t0", "memory" \
        );                                                              \
    }

#define ECALL5v(_name, _id, _type0, _arg0, _type1, _arg1, _type2, _arg2, _type3, _arg3, _type4, _arg4) \
    void _name(_type0 _arg0, _type1 _arg1, _type2 _arg2, _type3 _arg3, _type4 _arg4) \
    {                                                                   \
        register uint32_t a0 asm ("a0") = (uint32_t)_arg0;              \
        register uint32_t a1 asm ("a1") = (uint32_t)_arg1;              \
        register uint32_t a2 asm ("a2") = (uint32_t)_arg2;              \
        register uint32_t a3 asm ("a3") = (uint32_t)_arg3;              \
        register uint32_t a4 asm ("a4") = (uint32_t)_arg4;              \
        asm (                                                           \
             "li t0, %0\n"                                              \
             "ecall\n"                                                  \
             :: "i"(_id), "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4) : "t0", "memory" \
        );                                                              \
    }

#define ECALL5(_name, _id, _ret_type, _type0, _arg0, _type1, _arg1, _type2, _arg2, _type3, _arg3, _type4, _arg4) \
    _ret_type _name(_type0 _arg0, _type1 _arg1, _type2 _arg2, _type3 _arg3, _type4 _arg4) \
    {                                                                   \
        register uint32_t a0 asm ("a0") = (uint32_t)_arg0;              \
        register uint32_t a1 asm ("a1") = (uint32_t)_arg1;              \
        register uint32_t a2 asm ("a2") = (uint32_t)_arg2;              \
        register uint32_t a3 asm ("a3") = (uint32_t)_arg3;              \
        register uint32_t a4 asm ("a4") = (uint32_t)_arg4;              \
        _ret_type ret;                                                  \
        asm (                                                           \
             "li t0, %1\n"                                              \
             "ecall\n"                                                  \
             : "=r"(ret) : "i"(_id), "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4) : "t0", "memory" \
        );                                                              \
        return ret;                                                     \
    }

#define ECALL6(_name, _id, _ret_type, _type0, _arg0, _type1, _arg1, _type2, _arg2, _type3, _arg3, _type4, _arg4, _type5, _arg5) \
    _ret_type _name(_type0 _arg0, _type1 _arg1, _type2 _arg2, _type3 _arg3, _type4 _arg4, _type5 _arg5) \
    {                                                                   \
        register uint32_t a0 asm ("a0") = (uint32_t)_arg0;              \
        register uint32_t a1 asm ("a1") = (uint32_t)_arg1;              \
        register uint32_t a2 asm ("a2") = (uint32_t)_arg2;              \
        register uint32_t a3 asm ("a3") = (uint32_t)_arg3;              \
        register uint32_t a4 asm ("a4") = (uint32_t)_arg4;              \
        register uint32_t a5 asm ("a5") = (uint32_t)_arg5;              \
        _ret_type ret;                                                  \
        asm (                                                           \
             "li t0, %1\n"                                              \
             "ecall\n"                                                  \
             : "=r"(ret) : "i"(_id), "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5) : "t0", "memory" \
        );                                                              \
        return ret;                                                     \
    }

#define ECALL6v(_name, _id, _type0, _arg0, _type1, _arg1, _type2, _arg2, _type3, _arg3, _type4, _arg4, _type5, _arg5) \
    void _name(_type0 _arg0, _type1 _arg1, _type2 _arg2, _type3 _arg3, _type4 _arg4, _type5 _arg5) \
    {                                                                   \
        register uint32_t a0 asm ("a0") = (uint32_t)_arg0;              \
        register uint32_t a1 asm ("a1") = (uint32_t)_arg1;              \
        register uint32_t a2 asm ("a2") = (uint32_t)_arg2;              \
        register uint32_t a3 asm ("a3") = (uint32_t)_arg3;              \
        register uint32_t a4 asm ("a4") = (uint32_t)_arg4;              \
        register uint32_t a5 asm ("a5") = (uint32_t)_arg5;              \
        asm (                                                           \
             "li t0, %0\n"                                              \
             "ecall\n"                                                  \
             :: "i"(_id), "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5) : "t0", "memory" \
        );                                                              \
    }

#define ECALL8v(_name, _id, _type0, _arg0, _type1, _arg1, _type2, _arg2, _type3, _arg3, _type4, _arg4, _type5, _arg5, _type6, _arg6, _type7, _arg7) \
    void _name(_type0 _arg0, _type1 _arg1, _type2 _arg2, _type3 _arg3, _type4 _arg4, _type5 _arg5, _type6 _arg6, _type7 _arg7) \
    {                                                                   \
        register uint32_t a0 asm ("a0") = (uint32_t)_arg0;              \
        register uint32_t a1 asm ("a1") = (uint32_t)_arg1;              \
        register uint32_t a2 asm ("a2") = (uint32_t)_arg2;              \
        register uint32_t a3 asm ("a3") = (uint32_t)_arg3;              \
        register uint32_t a4 asm ("a4") = (uint32_t)_arg4;              \
        register uint32_t a5 asm ("a5") = (uint32_t)_arg5;              \
        register uint32_t a6 asm ("a6") = (uint32_t)_arg6;              \
        register uint32_t a7 asm ("a7") = (uint32_t)_arg7;              \
        asm (                                                           \
             "li t0, %0\n"                                              \
             "ecall\n"                                                  \
             :: "i"(_id), "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7) : "t0", "memory" \
        );                                                              \
    }

ECALL0(ecall_wait_button, ECALL_WAIT_BUTTON, int)
ECALL0(ecall_app_loading_stop, ECALL_LOADING_STOP, bool)
ECALL0v(ecall_screen_update, ECALL_SCREEN_UPDATE)
ECALL0v(ecall_ux_idle, ECALL_UX_IDLE)
ECALL1(ecall_strlen, ECALL_STRLEN, size_t, const char *, s)
ECALL1v(ecall_app_loading_start, ECALL_LOADING_START, const char *, status)
ECALL2(ecall_xrecv, ECALL_XRECV, size_t, uint8_t *, buffer, size_t, size)
ECALL2v(ecall_xsend, ECALL_XSEND, const uint8_t *, buffer, size_t, size)
ECALL3(ecall_ecfp_generate_pair, ECALL_CX_ECFP_GENERATE_PAIR, cx_err_t, cx_curve_t, curve, cx_ecfp_public_key_t *, pubkey, cx_ecfp_private_key_t *, privkey)
ECALL3(ecall_ecfp_get_pubkey, ECALL_CX_ECFP_GET_PUBKEY, cx_err_t, cx_curve_t, curve, cx_ecfp_public_key_t *, pubkey, const cx_ecfp_private_key_t *, privkey)
ECALL3(ecall_hash_final, ECALL_HASH_FINAL, bool, const cx_hash_id_t, hash_id, ctx_hash_guest_t *, ctx, uint8_t *, digest)
ECALL3(ecall_memset, ECALL_MEMSET, void *, void *, s, int, c, size_t, n)
ECALL3(ecall_memcpy, ECALL_MEMCPY, void *, void *, dest, const void *, src, size_t, n)
ECALL3v(ecall_sha256sum, ECALL_SHA256SUM, const uint8_t *, buffer, size_t, size, uint8_t *, digest)
ECALL3v(ecall_sha3_256, ECALL_CX_SHA3_256, const uint8_t *, buffer, size_t, size, uint8_t *, digest)
ECALL4(ecall_hash_update, ECALL_HASH_UPDATE, bool, const cx_hash_id_t, hash_id, ctx_hash_guest_t *, ctx, const uint8_t *, buffer, const size_t, size)
ECALL4(ecall_tostring256, ECALL_TOSTRING256, bool, const uint256_t *, number, const unsigned int, base, char *, out, size_t, len)
ECALL4v(ecall_bagl_draw_with_context, ECALL_BAGL_DRAW, packed_bagl_component_t *,component, const void *, context, unsigned short, context_length, unsigned char, context_encoding)
ECALL4v(ecall_mult, ECALL_MULT, uint8_t *, r, const uint8_t *, a, const uint8_t *, b, size_t, len)
ECALL5(ecall_derive_node_bip32, ECALL_DERIVE_NODE_BIP32, cx_err_t, cx_curve_t, curve, const unsigned int *, path, size_t, path_count, uint8_t *, private_key, uint8_t *, chain)
ECALL5v(ecall_bagl_hal_draw_rect, ECALL_UX_RECTANGLE, unsigned int, color, int, x, int, y, unsigned int, width, unsigned int, height)
ECALL5v(ecall_multm, ECALL_MULTM, uint8_t *, r, const uint8_t *, a, const uint8_t *, b, const uint8_t *, m, size_t, len)
ECALL6(ecall_ecdsa_sign, ECALL_ECDSA_SIGN, size_t, const cx_ecfp_private_key_t *, key, const int, mode, const cx_md_t, hash_id, const uint8_t *, hash, uint8_t *, sig, size_t, sig_len)
ECALL8v(ecall_bagl_hal_draw_bitmap_within_rect, ECALL_BAGL_DRAW_BITMAP, int, x, int, y, unsigned int, width, unsigned int, height, const unsigned int *, colors, unsigned int, bit_per_pixel, const unsigned char *, bitmap, unsigned int, bitmap_length_bits)

__attribute__((noreturn)) void ecall_exit(int status)
{
    register uint32_t a0 asm ("a0") = (uint32_t)status;
    asm volatile (
        "li t0, %0\n"
        "ecall\n"
        :: "i"(ECALL_EXIT), "r"(a0) : "t0"
    );

    while (1);
}

__attribute__((noreturn)) void ecall_fatal(char *msg)
{
    register uint32_t a0 asm ("a0") = (uint32_t)msg;
    asm volatile (
        "li t0, %0\n"
        "ecall\n"
        :: "i"(ECALL_FATAL), "r"(a0) : "t0"
    );

    while (1);
}

/* clang-format on */

void *memset(void *s, int c, size_t n) __attribute__((alias("ecall_memset")));

void *memcpy(void *dest, const void *src, size_t n) __attribute__((alias("ecall_memcpy")));

size_t strlen(const char *s) __attribute__((alias("ecall_strlen")));
