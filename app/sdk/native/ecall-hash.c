#define MIN(a, b) ((a) < (b) ? (a) : (b))

union cx_hash_ctx_u {
    cx_sha256_t sha256;
    cx_sha3_t sha3;
    cx_hash_t header;
};

void ecall_sha256sum(const uint8_t *buffer, size_t size, uint8_t *digest)
{
    /* XXX */
    memset(digest, 'a', 32);
}

void ecall_sha3_256(const uint8_t *buffer, size_t size, uint8_t *digest)
{
    cx_sha3_t ctx;

    cx_keccak_init(&ctx, 256);
    sys_cx_hash((cx_hash_t *)&ctx, CX_LAST, buffer, size, digest, 32);
}

static bool restore_ctx_from_guest(const cx_hash_id_t hash_id,
                                   ctx_hash_guest_t *guest,
                                   union cx_hash_ctx_u *ctx)
{
    switch (hash_id) {
    case HASH_ID_SHA3_256:
        cx_keccak_init(&ctx->sha3, 256);
        ctx->sha3.blen = guest->sha3.blen;
        memcpy(&ctx->sha3.block, guest->sha3.block, sizeof(ctx->sha3.block));
        memcpy(&ctx->sha3.acc, guest->sha3.acc, sizeof(ctx->sha3.acc));
        break;
    case HASH_ID_SHA256:
        cx_sha256_init(&ctx->sha256);
        ctx->sha256.blen = guest->sha256.blen;
        memcpy(&ctx->sha256.block, guest->sha256.block, sizeof(ctx->sha256.block));
        memcpy(&ctx->sha256.acc, guest->sha256.acc, sizeof(ctx->sha256.acc));
    default:
        return false;
    }

    return true;
}

static bool save_ctx_from_host(const cx_hash_id_t hash_id,
                               ctx_hash_guest_t *guest,
                               union cx_hash_ctx_u *ctx)
{
    switch (hash_id) {
    case HASH_ID_SHA3_256:
        guest->sha3.blen = ctx->sha3.blen;
        memcpy(guest->sha3.block, &ctx->sha3.block, sizeof(ctx->sha3.block));
        memcpy(guest->sha3.acc, &ctx->sha3.acc, sizeof(ctx->sha3.acc));
        break;
    case HASH_ID_SHA256:
        guest->sha256.blen = ctx->sha256.blen;
        memcpy(guest->sha256.block, &ctx->sha256.block, sizeof(ctx->sha256.block));
        memcpy(guest->sha256.acc, &ctx->sha256.acc, sizeof(ctx->sha256.acc));
    default:
        return false;
    }

    return true;
}

bool ecall_hash_update(const cx_hash_id_t hash_id,
                       ctx_hash_guest_t *guest_ctx,
                       const uint8_t *buffer,
                       const size_t size)
{
    union cx_hash_ctx_u ctx;

    if (!restore_ctx_from_guest(hash_id, guest_ctx, &ctx)) {
        return false;
    }

    /* XXX: return value? */
    sys_cx_hash((cx_hash_t *)&ctx, 0, buffer, size, NULL, 0);

    return save_ctx_from_host(hash_id, guest_ctx, &ctx);
}

bool ecall_hash_final(const cx_hash_id_t hash_id, ctx_hash_guest_t *guest_ctx, uint8_t *digest)
{
    union cx_hash_ctx_u ctx;
    size_t hash_len;

    if (!restore_ctx_from_guest(hash_id, guest_ctx, &ctx)) {
        return false;
    }

    switch (hash_id) {
    case HASH_ID_SHA3_256:
        hash_len = CX_SHA256_SIZE;
        break;
    case HASH_ID_SHA256:
        hash_len = CX_SHA256_SIZE;
        break;
    default:
        return false;
    }

    /* XXX: return value? */
    sys_cx_hash(&ctx.header, CX_LAST, NULL, 0, digest, hash_len);

    /* no need to save the context */

    return true;
}
