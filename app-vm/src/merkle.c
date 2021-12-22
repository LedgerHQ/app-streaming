#include <string.h>

#include "merkle.h"

#include "cx.h"

struct entry_s {
    uint8_t data[8]; // addr, iv
} __attribute__((packed));

static uint8_t root_hash[CX_SHA256_SIZE];
static size_t n;
static cx_sha256_t hash_ctx;
static struct entry_s last_entry;

static void hash_entry(struct entry_s *entry, uint8_t *hash)
{
    cx_sha256_init_no_throw(&hash_ctx);

    cx_hash_no_throw((cx_hash_t *)&hash_ctx, 0, (uint8_t *)"\x00", 1, NULL, 0);
    cx_hash_no_throw((cx_hash_t *)&hash_ctx, CX_LAST,
                     (uint8_t *)entry, sizeof(*entry),
                     hash, CX_SHA256_SIZE);
}

static void hash_nodes(uint8_t *left, uint8_t *right, uint8_t *hash)
{
    cx_sha256_init_no_throw(&hash_ctx);

    cx_hash_no_throw((cx_hash_t *)&hash_ctx, 0, (uint8_t *)"\x01", sizeof(uint8_t), NULL, 0);
    cx_hash_no_throw((cx_hash_t *)&hash_ctx, 0, left, CX_SHA256_SIZE, NULL, 0);
    cx_hash_no_throw((cx_hash_t *)&hash_ctx, CX_LAST,
                     right, CX_SHA256_SIZE,
                     hash, CX_SHA256_SIZE);
}

static void proof_hash(struct entry_s *entry, struct proof_s *proof, size_t count, uint8_t *digest)
{
    if (entry != NULL) {
        hash_entry(entry, digest);
    }

    for (size_t i = 0; i < count; i++) {
        uint8_t *left, *right;

        if (proof->op == 'L') {
            left = proof->digest;
            right = digest;
        } else {
            left = digest;
            right = proof->digest;
        }

        hash_nodes(left, right, digest);

        proof++;
    }
}

static size_t bit_count(uint32_t x)
{
    size_t count = 0;

    while (x) {
        count += x & 1;
        x >>= 1;
    }

    return count;
}

static void log_merkle_root_hash(void)
{
    char hex[16] = "0123456789abcdef";
    size_t i;
    char buf[3];

    buf[2] = '\x00';

    err("\nroot hash: ");

    for (i = 0; i < sizeof(root_hash); i++) {
        buf[0] = hex[(root_hash[i] >> 4) & 0xf];
        buf[1] = hex[root_hash[i] & 0xf];
        err(buf);
    }

    err("\n");
}

bool merkle_insert(struct entry_s *entry, struct proof_s *proof, size_t count)
{
    /* XXX: check that n doesn't overflow */

    if (n == 0) {
        hash_entry(entry, root_hash);
        memcpy(&last_entry, entry, sizeof(*entry));
        n++;
        return true;
    }

    struct proof_s tmp_proof;
    size_t tree_level = count - (bit_count(n) - 1);

    /* check against integer overflow */
    if (tree_level > count) {
        return false;
    }

    proof_hash(&last_entry, proof, count, tmp_proof.digest);
    if (memcmp(tmp_proof.digest, root_hash, sizeof(root_hash)) != 0) {
        fatal("invalid proof hash\n");
        return false;
    }

    tmp_proof.op = 'L';
    proof_hash(&last_entry, proof, tree_level, tmp_proof.digest);

    /* compute new root hash */
    proof_hash(entry, &tmp_proof, 1, root_hash);
    proof_hash(NULL, proof + tree_level, count - tree_level, root_hash);

    memcpy(&last_entry, entry, sizeof(*entry));
    n++;

    //log_merkle_root_hash();

    return true;
}

bool merkle_update(struct entry_s *old_entry, struct entry_s *entry, struct proof_s *proof, size_t count)
{
    if (!merkle_verify_proof(old_entry, proof, count)) {
        fatal("invalid update proof hash\n");
        return false;
    }

    proof_hash(entry, (struct proof_s *)proof, count, root_hash);

    //log_merkle_root_hash();

    /* update last entry if required */
    if (memcmp(&last_entry, old_entry, sizeof(last_entry)) == 0) {
        memcpy(&last_entry, entry, sizeof(last_entry));
    }

    return true;
}

bool merkle_verify_proof(struct entry_s *entry, uint8_t *proof, size_t count)
{
    uint8_t digest[CX_SHA256_SIZE];

    proof_hash(entry, (struct proof_s *)proof, count, digest);

    return memcmp(digest, root_hash, sizeof(root_hash)) == 0;
}

void init_merkle_tree(uint8_t *root_hash_init, size_t merkle_tree_size, struct entry_s *last_entry_init)
{
    memcpy(root_hash, root_hash_init, sizeof(root_hash));
    memcpy(&last_entry, last_entry_init, sizeof(last_entry));
    n = merkle_tree_size;

    //log_merkle_root_hash();
}
