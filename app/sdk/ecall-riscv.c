#include "ecall.h"
#include "ecall-nr.h"

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
ECALL0v(ecall_screen_update, ECALL_SCREEN_UPDATE)
ECALL2(ecall_xrecv, ECALL_XRECV, size_t, uint8_t *, buffer, size_t, size)
ECALL2v(ecall_xsend, ECALL_XSEND, const uint8_t *, buffer, size_t, size)
ECALL3v(ecall_sha256sum, ECALL_SHA256SUM, const uint8_t *, buffer, size_t, size, uint8_t *, digest)
ECALL4v(ecall_bagl_draw_with_context, ECALL_BAGL_DRAW, packed_bagl_component_t *,component, const void *, context, unsigned short, context_length, unsigned char, context_encoding)
ECALL5v(ecall_bagl_hal_draw_rect, ECALL_UX_RECTANGLE, unsigned int, color, int, x, int, y, unsigned int, width, unsigned int, height)
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

void sha256sum(const uint8_t *buffer, size_t size, uint8_t *digest) \
    __attribute__((alias("ecall_sha256sum")));

void screen_update(void) \
    __attribute__((alias("ecall_screen_update")));

void xsend(const uint8_t *buffer, size_t size) \
    __attribute__((alias("ecall_xsend")));

int wait_button(void) \
    __attribute__((alias("ecall_wait_button")));
