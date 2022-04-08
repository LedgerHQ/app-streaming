/**
 * Wrappers for speculos syscalls and cxlib functions.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "speculos.h"

cx_err_t cx_bn_alloc(cx_bn_t *x, size_t nbytes)
{
    return sys_cx_bn_alloc(x, nbytes);
}

cx_err_t cx_bn_alloc_init(cx_bn_t *x, size_t nbytes, const uint8_t *value, size_t value_nbytes)
{
    return sys_cx_bn_alloc_init(x, nbytes, value, value_nbytes);
}

cx_err_t cx_bn_cmp(const cx_bn_t bn_a, const cx_bn_t bn_b, int *diff)
{
    return sys_cx_bn_cmp(bn_a, bn_b, diff);
}

cx_err_t cx_bn_destroy(cx_bn_t *bn_x)
{
    return sys_cx_bn_destroy(bn_x);
}

cx_err_t cx_bn_export(const cx_bn_t bn_x, uint8_t *bytes, size_t nbytes)
{
    return sys_cx_bn_export(bn_x, bytes, nbytes);
}

cx_err_t cx_bn_init(cx_bn_t bn_x, const uint8_t *bytes, size_t nbytes)
{
    return sys_cx_bn_init(bn_x, bytes, nbytes);
}

cx_err_t cx_bn_lock(size_t word_size, uint32_t flags)
{
    return sys_cx_bn_lock(word_size, flags);
}

cx_err_t cx_bn_mod_add(cx_bn_t bn_r, const cx_bn_t bn_a, const cx_bn_t bn_b, const cx_bn_t bn_m)
{
    return sys_cx_bn_mod_add(bn_r, bn_a, bn_b, bn_m);
}

cx_err_t cx_bn_mod_mul(cx_bn_t bn_r, const cx_bn_t bn_a, const cx_bn_t bn_b, const cx_bn_t bn_m)
{
    return sys_cx_bn_mod_mul(bn_r, bn_a, bn_b, bn_m);
}

cx_err_t cx_bn_rand(cx_bn_t bn_x)
{
    return sys_cx_bn_rand(bn_x);
}

cx_err_t cx_bn_reduce(cx_bn_t bn_r, const cx_bn_t bn_d, const cx_bn_t bn_n)
{
    return sys_cx_bn_reduce(bn_r, bn_d, bn_n);
}

cx_err_t cx_bn_unlock(void)
{
    return sys_cx_bn_unlock();
}

cx_err_t cx_ecdomain_generator_bn(cx_curve_t cv, cx_ecpoint_t *P)
{
    return sys_cx_ecdomain_generator_bn(cv, P);
}

cx_err_t cx_ecdomain_parameter_bn(cx_curve_t cv, cx_curve_dom_param_t id, cx_bn_t p)
{
    return sys_cx_ecdomain_parameter_bn(cv, id, p);
}

cx_err_t cx_ecdomain_parameters_length(cx_curve_t cv, size_t *length)
{
    return sys_cx_ecdomain_parameters_length(cv, length);
}

cx_err_t cx_ecpoint_add(cx_ecpoint_t *R, const cx_ecpoint_t *P, const cx_ecpoint_t *Q)
{
    return sys_cx_ecpoint_add(R, P, Q);
}

cx_err_t cx_ecpoint_alloc(cx_ecpoint_t *P, cx_curve_t cv)
{
    return sys_cx_ecpoint_alloc(P, cv);
}

cx_err_t cx_ecpoint_cmp(const cx_ecpoint_t *P, const cx_ecpoint_t *Q, bool *is_equal)
{
    return sys_cx_ecpoint_cmp(P, Q, is_equal);
}

cx_err_t cx_ecpoint_compress(const cx_ecpoint_t *P,
                             uint8_t *xy_compressed,
                             size_t xy_compressed_len,
                             uint32_t *sign)
{
    return sys_cx_ecpoint_compress(P, xy_compressed, xy_compressed_len, sign);
}

cx_err_t cx_ecpoint_decompress(cx_ecpoint_t *P,
                               const uint8_t *xy_compressed,
                               size_t xy_compressed_len,
                               uint32_t sign)
{
    return sys_cx_ecpoint_decompress(P, xy_compressed, xy_compressed_len, sign);
}

cx_err_t cx_ecpoint_destroy(cx_ecpoint_t *P)
{
    return sys_cx_ecpoint_destroy(P);
}

cx_err_t cx_ecpoint_double_scalarmul(cx_ecpoint_t *R,
                                     cx_ecpoint_t *P,
                                     cx_ecpoint_t *Q,
                                     const uint8_t *k,
                                     size_t k_len,
                                     const uint8_t *r,
                                     size_t r_len)
{
    return sys_cx_ecpoint_double_scalarmul(R, P, Q, k, k_len, r, r_len);
}

cx_err_t cx_ecpoint_export(const cx_ecpoint_t *P,
                           uint8_t *x,
                           size_t x_len,
                           uint8_t *y,
                           size_t y_len)
{
    return sys_cx_ecpoint_export(P, x, x_len, y, y_len);
}

cx_err_t cx_ecpoint_init(cx_ecpoint_t *P,
                         const uint8_t *x,
                         size_t x_len,
                         const uint8_t *y,
                         size_t y_len)
{
    return sys_cx_ecpoint_init(P, x, x_len, y, y_len);
}

cx_err_t cx_ecpoint_neg(cx_ecpoint_t *P)
{
    return sys_cx_ecpoint_neg(P);
}

cx_err_t cx_ecpoint_rnd_scalarmul(cx_ecpoint_t *P, const uint8_t *k, size_t k_len)
{
    return sys_cx_ecpoint_rnd_scalarmul(P, k, k_len);
}

cx_err_t cx_ecpoint_scalarmul(cx_ecpoint_t *P, const uint8_t *k, size_t k_len)
{
    return sys_cx_ecpoint_scalarmul(P, k, k_len);
}

void os_perso_derive_node_bip32(cx_curve_t curve,
                                const unsigned int *path,
                                unsigned int pathLength,
                                unsigned char *privateKey,
                                unsigned char *chain)
{
    sys_os_perso_derive_node_bip32(curve, path, pathLength, privateKey, chain);
}

cx_err_t cx_bn_add(cx_bn_t r, const cx_bn_t a, const cx_bn_t b)
{
    return sys_cx_bn_add(r, a, b);
}

cx_err_t cx_bn_is_prime(const cx_bn_t n, bool *prime)
{
    return sys_cx_bn_is_prime(n, prime);
}

cx_err_t cx_bn_mod_invert_nprime(cx_bn_t r, const cx_bn_t a, const cx_bn_t n)
{
    return sys_cx_bn_mod_invert_nprime(r, a, n);
}

cx_err_t cx_bn_mod_pow2(cx_bn_t r,
                        const cx_bn_t a,
                        const uint8_t *e,
                        uint32_t e_len,
                        const cx_bn_t n)
{
    return sys_cx_bn_mod_pow2(r, a, e, e_len, n);
}

cx_err_t cx_bn_mod_sub(cx_bn_t r, const cx_bn_t a, const cx_bn_t b, const cx_bn_t n)
{
    return sys_cx_bn_mod_sub(r, a, b, n);
}

cx_err_t cx_bn_mod_u32_invert(cx_bn_t r, uint32_t a, cx_bn_t n)
{
    return sys_cx_bn_mod_u32_invert(r, a, n);
}

cx_err_t cx_bn_mul(cx_bn_t r, const cx_bn_t a, const cx_bn_t b)
{
    return sys_cx_bn_mul(r, a, b);
}

cx_err_t cx_bn_next_prime(cx_bn_t n)
{
    return sys_cx_bn_next_prime(n);
}

cx_err_t cx_bn_sub(cx_bn_t r, const cx_bn_t a, const cx_bn_t b)
{
    return sys_cx_bn_sub(r, a, b);
}

void os_longjmp(unsigned int exception)
{
    fprintf(stderr, "os_longjmp() called\n");
    fprintf(stderr, "This shouldn't never happen, there's a bug somewhere\n");
    exit(1);
}
