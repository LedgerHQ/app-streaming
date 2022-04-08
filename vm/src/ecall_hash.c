#include <string.h>

#include "ecall.h"
#include "ecall_hash.h"
#include "page.h"

#include "cx.h"

union cx_hash_ctx_u {
    cx_sha256_t sha256;
    cx_sha3_t sha3;
    cx_hash_t header;
};

void sys_sha256sum(guest_pointer_t p_data, size_t size, guest_pointer_t p_digest)
{
    uint8_t digest[CX_SHA256_SIZE];
    cx_sha256_t ctx;

    cx_sha256_init_no_throw(&ctx);

    /* compute digest over the guest data */
    while (size > 0) {
        const size_t n = BUFFER_MIN_SIZE(p_data.addr, size);
        const uint8_t *buffer = get_buffer(p_data.addr, n, false);

        if (size - n != 0) {
            cx_hash_no_throw((cx_hash_t *)&ctx, 0, buffer, n, NULL, 0);
        } else {
            cx_hash_no_throw((cx_hash_t *)&ctx, CX_LAST, buffer, n, digest, sizeof(digest));
        }

        p_data.addr += n;
        size -= n;
    }

    /* copy digest to the guest addr */
    size = sizeof(digest);
    while (size > 0) {
        const size_t n = BUFFER_MIN_SIZE(p_digest.addr, size);
        uint8_t *buffer = get_buffer(p_digest.addr, n, true);

        memcpy(buffer, digest + sizeof(digest) - size, n);

        p_digest.addr += n;
        size -= n;
    }
}

void sys_sha3_256(guest_pointer_t p_buffer, size_t size, guest_pointer_t p_digest)
{
    cx_sha3_t ctx;

    cx_keccak_init(&ctx, 256);
    while (size > 0) {
        const size_t n = BUFFER_MIN_SIZE(p_buffer.addr, size);
        const uint8_t *buffer = get_buffer(p_buffer.addr, n, false);

        cx_hash(&ctx.header, 0, buffer, n, NULL, 0);

        p_buffer.addr += n;
        size -= n;
    }

    uint8_t digest[32];
    cx_hash((cx_hash_t *)&ctx, CX_LAST, NULL, 0, digest, sizeof(digest));

    copy_host_buffer(p_digest, digest, sizeof(digest));
}

static bool restore_ctx_from_guest(const cx_hash_id_t hash_id, guest_pointer_t p_ctx, union cx_hash_ctx_u *ctx)
{
    ctx_hash_guest_t guest;

    switch (hash_id) {
    case HASH_ID_SHA3_256:
        copy_guest_buffer(p_ctx, &guest.sha3, sizeof(guest.sha3));
        cx_keccak_init(&ctx->sha3, 256);
        ctx->sha3.blen = guest.sha3.blen;
        memcpy(&ctx->sha3.block, guest.sha3.block, sizeof(ctx->sha3.block));
        memcpy(&ctx->sha3.acc, guest.sha3.acc, sizeof(ctx->sha3.acc));
        break;
    case HASH_ID_SHA256:
        copy_guest_buffer(p_ctx, &guest.sha256, sizeof(guest.sha256));
        cx_sha256_init_no_throw(&ctx->sha256);
        ctx->sha256.blen = guest.sha256.blen;
        memcpy(&ctx->sha256.block, guest.sha256.block, sizeof(ctx->sha256.block));
        memcpy(&ctx->sha256.acc, guest.sha256.acc, sizeof(ctx->sha256.acc));
    default:
        return false;
    }

    return true;
}

static bool save_ctx_from_host(const cx_hash_id_t hash_id, guest_pointer_t p_ctx, union cx_hash_ctx_u *ctx)
{
    ctx_hash_guest_t guest;

    switch (hash_id) {
    case HASH_ID_SHA3_256:
        guest.sha3.blen = ctx->sha3.blen;
        memcpy(guest.sha3.block, &ctx->sha3.block, sizeof(ctx->sha3.block));
        memcpy(guest.sha3.acc, &ctx->sha3.acc, sizeof(ctx->sha3.acc));
        copy_host_buffer(p_ctx, &guest.sha3, sizeof(guest.sha3));
        break;
    case HASH_ID_SHA256:
        guest.sha256.blen = ctx->sha256.blen;
        memcpy(guest.sha256.block, &ctx->sha256.block, sizeof(ctx->sha256.block));
        memcpy(guest.sha256.acc, &ctx->sha256.acc, sizeof(ctx->sha256.acc));
        copy_host_buffer(p_ctx, &guest.sha256, sizeof(guest.sha256));
    default:
        return false;
    }

    return true;
}

bool sys_hash_update(const cx_hash_id_t hash_id, guest_pointer_t p_ctx, guest_pointer_t p_buffer, size_t size)
{
    union cx_hash_ctx_u ctx;

    if (!restore_ctx_from_guest(hash_id, p_ctx, &ctx)) {
        return false;
    }

    while (size > 0) {
        const size_t n = BUFFER_MIN_SIZE(p_buffer.addr, size);
        const uint8_t *buffer = get_buffer(p_buffer.addr, n, false);

        cx_hash((cx_hash_t *)&ctx, 0, buffer, n, NULL, 0);

        p_buffer.addr += n;
        size -= n;
    }

    return save_ctx_from_host(hash_id, p_ctx, &ctx);
}

bool sys_hash_final(const cx_hash_id_t hash_id, guest_pointer_t p_ctx, guest_pointer_t p_digest)
{
    union cx_hash_ctx_u ctx;
    size_t hash_len;

    if (!restore_ctx_from_guest(hash_id, p_ctx, &ctx)) {
        return false;
    }

    switch (hash_id) {
    case HASH_ID_SHA3_256: hash_len = CX_SHA256_SIZE; break;
    case HASH_ID_SHA256: hash_len = CX_SHA256_SIZE; break;
    default: return false;
    }

    uint8_t digest[CX_SHA512_SIZE];
    cx_hash(&ctx.header, CX_LAST, NULL, 0, digest, hash_len);

    copy_host_buffer(p_digest, digest, hash_len);

    /* no need to save the context */

    return true;
}
