#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "cx.h"

#include "sdk/api/ecall-params.h"

struct rv_cpu;

typedef struct guest_pointer_s {
    uint32_t addr;
} guest_pointer_t;

/* ECALL return value */
typedef union eret_u {
    uint32_t addr;
    bool boolean;
    size_t size;
} eret_t;

#define ERET(reg) ((eret_t *)&cpu->regs[reg])

/* Guest Pointer */
#define GP(reg)   ((const guest_pointer_t){ .addr = cpu->regs[reg] })

bool copy_guest_buffer(guest_pointer_t p_src, void *buf, size_t size);
bool copy_host_buffer(guest_pointer_t p_dst, void *buf, size_t size);
uint8_t *get_buffer(const uint32_t addr, const size_t size, const bool writeable);

bool sys_xsend(guest_pointer_t p_buf, size_t size);
bool sys_xrecv(eret_t *eret, guest_pointer_t p_buf, size_t size);
void sys_fatal(guest_pointer_t p_msg);
void sys_exit(uint32_t code);
void sys_app_loading_start(guest_pointer_t p_status);
bool sys_app_loading_stop(void);
void sys_ux_rectangle(uint32_t color, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void sys_screen_update(void);
void sys_ux_bitmap(int x, int y, unsigned int width, unsigned int height, /*unsigned int color_count,*/ guest_pointer_t p_colors, unsigned int bit_per_pixel, guest_pointer_t p_bitmap, unsigned int bitmap_length_bits);
int sys_wait_button(void);
void sys_bagl_draw_with_context(guest_pointer_t p_component, guest_pointer_t p_context, size_t context_length, int context_encoding);
void sys_ux_idle(void);
bool sys_memset(eret_t *eret, guest_pointer_t p_s, int c, size_t size);
bool sys_memcpy(eret_t *eret, guest_pointer_t p_dst, guest_pointer_t p_src, size_t size);
bool sys_strlen(eret_t *eret, guest_pointer_t p_s);
bool sys_strnlen(eret_t *eret, guest_pointer_t p_s, size_t maxlen);

cx_err_t sys_derive_node_bip32(cx_curve_t curve, guest_pointer_t p_path, size_t path_count, guest_pointer_t p_private_key, guest_pointer_t p_chain);
cx_err_t sys_ecfp_generate_pair(cx_curve_t curve, guest_pointer_t p_pubkey, guest_pointer_t p_privkey);
cx_err_t sys_ecfp_get_pubkey(cx_curve_t curve, guest_pointer_t p_pubkey, guest_pointer_t p_privkey);
size_t sys_ecdsa_sign(const guest_pointer_t p_key, const int mode,
                      const cx_md_t hash_id, const guest_pointer_t p_hash,
                      guest_pointer_t p_sig, size_t sig_len);
void sys_mult(guest_pointer_t p_r, guest_pointer_t p_a, guest_pointer_t p_b, size_t len);
void sys_multm(guest_pointer_t p_r, guest_pointer_t p_a, guest_pointer_t p_b, guest_pointer_t p_m, size_t len);
bool sys_tostring256(const guest_pointer_t p_number, const unsigned int base, guest_pointer_t p_out, size_t len);

bool ecall(struct rv_cpu *cpu);
