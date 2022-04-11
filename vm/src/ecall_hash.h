#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "sdk/api/ecall-params.h"

bool sys_sha256sum(guest_pointer_t p_data, size_t size, guest_pointer_t p_digest);
bool sys_sha3_256(const guest_pointer_t p_buffer, size_t size, const guest_pointer_t p_digest);
bool sys_hash_update(const cx_hash_id_t hash_id, guest_pointer_t p_ctx, guest_pointer_t p_buffer, size_t size, uint32_t *ret);
bool sys_hash_final(const cx_hash_id_t hash_id, guest_pointer_t p_ctx, guest_pointer_t p_addr);
