#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "cx.h"

struct proof_s {
    uint8_t op;
    uint8_t digest[32];
} __attribute__((packed));

/*
 * Merkle Tree node.
 */
struct entry_s {
    union {
        uint8_t data[8];
        struct {
            uint32_t addr;
            uint32_t iv;
        };
    };
} __attribute__((packed));

struct merkle_tree_ctx_s {
    uint8_t root_hash[CX_SHA256_SIZE];
    size_t n;
    struct entry_s last_entry;
};

bool merkle_insert(const struct entry_s *entry, const struct proof_s *proof, size_t count);
bool merkle_update(const struct entry_s *old_entry,
                   const struct entry_s *entry,
                   const struct proof_s *proof,
                   size_t count);
bool merkle_verify_proof(const struct entry_s *entry, const struct proof_s *proof, size_t count);
void init_merkle_tree(const uint8_t *root_hash_init,
                      size_t merkle_tree_size,
                      const struct entry_s *last_entry_init);
