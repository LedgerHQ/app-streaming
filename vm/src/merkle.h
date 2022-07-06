#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "cx.h"
#include "page.h"

struct proof_s {
    uint8_t op;
    uint8_t digest[CX_SHA256_SIZE];
} __attribute__((packed));

/*
 * Merkle Tree leaf.
 */
struct entry_s {
    union {
        uint8_t data[sizeof(struct page_s)];
        struct page_s page;
    };
} __attribute__((packed));

struct merkle_tree_ctx_s {
    uint8_t root_digest[CX_SHA256_SIZE];
    uint8_t last_entry_digest[CX_SHA256_SIZE];
    size_t n;
};

bool merkle_insert(const struct entry_s *entry, const struct proof_s *proof, size_t count);
bool merkle_update(const struct entry_s *old_entry,
                   const struct entry_s *entry,
                   const struct proof_s *proof,
                   size_t count);
bool merkle_verify_proof(const struct entry_s *entry, const struct proof_s *proof, size_t count);
void init_merkle_tree(const uint8_t *root_digest_init,
                      size_t merkle_tree_size,
                      const uint8_t *last_entry_digest_init);
