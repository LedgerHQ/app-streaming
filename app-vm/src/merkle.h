#pragma once

#include <stdbool.h>
#include <stdint.h>

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

bool merkle_insert(struct entry_s *entry, struct proof_s *proof, size_t count);
bool merkle_update(struct entry_s *old_entry, struct entry_s *entry, struct proof_s *proof, size_t count);
bool merkle_verify_proof(struct entry_s *entry, struct proof_s *proof, size_t count);
void init_merkle_tree(uint8_t *root_hash_init, size_t merkle_tree_size, struct entry_s *last_entry_init);
