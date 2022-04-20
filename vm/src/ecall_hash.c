#include <string.h>

#include "ecall.h"
#include "ecall_hash.h"
#include "error.h"
#include "page.h"

#include "cx.h"

union cx_hash_ctx_u {
    cx_ripemd160_t ripemd160;
    cx_sha256_t sha256;
    cx_sha3_t sha3;
    cx_hash_t header;
};

static bool restore_ctx_from_guest(eret_t *eret, const cx_hash_id_t hash_id, guest_pointer_t p_ctx, union cx_hash_ctx_u *ctx)
{
    ctx_hash_guest_t guest;

    eret->success = true;

    switch (hash_id) {
    case HASH_ID_RIPEMD160:
        if (!copy_guest_buffer(p_ctx, &guest.ripemd160, sizeof(guest.ripemd160))) {
            return false;
        }
        cx_ripemd160_init_no_throw(&ctx->ripemd160);
        if (guest.ripemd160.initialized) {
            ctx->ripemd160.blen = guest.ripemd160.blen; /* XXX: check */
            memcpy(&ctx->ripemd160.block, guest.ripemd160.block, sizeof(ctx->ripemd160.block));
            memcpy(&ctx->ripemd160.acc, guest.ripemd160.acc, sizeof(ctx->ripemd160.acc));
        }
        break;
    case HASH_ID_SHA3_256:
        if (!copy_guest_buffer(p_ctx, &guest.sha3, sizeof(guest.sha3))) {
            return false;
        }
        cx_keccak_init_no_throw(&ctx->sha3, 256);
        if (guest.sha3.initialized) {
            ctx->sha3.blen = guest.sha3.blen;
            memcpy(&ctx->sha3.block, guest.sha3.block, sizeof(ctx->sha3.block));
            memcpy(&ctx->sha3.acc, guest.sha3.acc, sizeof(ctx->sha3.acc));
        }
        break;
    case HASH_ID_SHA256:
        if (!copy_guest_buffer(p_ctx, &guest.sha256, sizeof(guest.sha256))) {
            return false;
        }
        cx_sha256_init_no_throw(&ctx->sha256);
        if (guest.sha256.initialized) {
            ctx->sha256.blen = guest.sha256.blen;
            memcpy(&ctx->sha256.block, guest.sha256.block, sizeof(ctx->sha256.block));
            memcpy(&ctx->sha256.acc, guest.sha256.acc, sizeof(ctx->sha256.acc));
        }
        break;
    default:
        eret->success = false;
        break;
    }

    return true;
}

static bool save_ctx_from_host(eret_t *eret, const cx_hash_id_t hash_id, guest_pointer_t p_ctx, union cx_hash_ctx_u *ctx)
{
    ctx_hash_guest_t guest;

    eret->success = true;

    switch (hash_id) {
    case HASH_ID_RIPEMD160:
        guest.ripemd160.initialized = true;
        guest.ripemd160.blen = ctx->ripemd160.blen;
        memcpy(guest.ripemd160.block, &ctx->ripemd160.block, sizeof(ctx->ripemd160.block));
        memcpy(guest.ripemd160.acc, &ctx->ripemd160.acc, sizeof(ctx->ripemd160.acc));
        if (!copy_host_buffer(p_ctx, &guest.ripemd160, sizeof(guest.ripemd160))) {
            return false;
        }
        break;
    case HASH_ID_SHA3_256:
        guest.sha3.initialized = true;
        guest.sha3.blen = ctx->sha3.blen;
        memcpy(guest.sha3.block, &ctx->sha3.block, sizeof(ctx->sha3.block));
        memcpy(guest.sha3.acc, &ctx->sha3.acc, sizeof(ctx->sha3.acc));
        if (!copy_host_buffer(p_ctx, &guest.sha3, sizeof(guest.sha3))) {
            return false;
        }
        break;
    case HASH_ID_SHA256:
        guest.sha256.initialized = true;
        guest.sha256.blen = ctx->sha256.blen;
        memcpy(guest.sha256.block, &ctx->sha256.block, sizeof(ctx->sha256.block));
        memcpy(guest.sha256.acc, &ctx->sha256.acc, sizeof(ctx->sha256.acc));
        if (!copy_host_buffer(p_ctx, &guest.sha256, sizeof(guest.sha256))) {
            return false;
        }
        break;
    default:
        eret->success = false;
        break;
    }

    return true;
}

/**
 * @return true on success, false otherwise
 */
bool sys_hash_update(eret_t *eret, const cx_hash_id_t hash_id, guest_pointer_t p_ctx, guest_pointer_t p_buffer, size_t size)
{
    union cx_hash_ctx_u ctx;

    if (!restore_ctx_from_guest(eret, hash_id, p_ctx, &ctx)) {
        return false;
    }

    if (!eret->success) {
        return true;
    }

    while (size > 0) {
        const size_t n = BUFFER_MIN_SIZE(p_buffer.addr, size);
        const uint8_t *buffer = get_buffer(p_buffer.addr, n, false);
        if (buffer == NULL) {
            return false;
        }

        if (cx_hash_no_throw((cx_hash_t *)&ctx, 0, buffer, n, NULL, 0) != CX_OK) {
            fatal("error in sys_hash_update");
        }

        p_buffer.addr += n;
        size -= n;
    }

    return save_ctx_from_host(eret, hash_id, p_ctx, &ctx);
}

bool sys_hash_final(eret_t *eret, const cx_hash_id_t hash_id, guest_pointer_t p_ctx, guest_pointer_t p_digest)
{
    union cx_hash_ctx_u ctx;
    size_t hash_len;

    if (!restore_ctx_from_guest(eret, hash_id, p_ctx, &ctx)) {
        return false;
    }

    if (!eret->success) {
        return true;
    }

    switch (hash_id) {
    case HASH_ID_RIPEMD160: hash_len = CX_RIPEMD160_SIZE; break;
    case HASH_ID_SHA3_256: hash_len = CX_SHA256_SIZE; break;
    case HASH_ID_SHA256: hash_len = CX_SHA256_SIZE; break;
    default: return false;
    }

    uint8_t digest[CX_SHA512_SIZE];
    if (cx_hash_no_throw(&ctx.header, CX_LAST, NULL, 0, digest, hash_len) != CX_OK) {
        fatal("error in sys_hash_final");
        return false;
    }

    if (!copy_host_buffer(p_digest, digest, hash_len)) {
        return false;
    }

    /* no need to save the context */

    return true;
}
