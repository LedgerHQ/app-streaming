#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "sdk/api/ecall-params.h"

bool sys_hash_update(eret_t *ret, const cx_hash_id_t hash_id, guest_pointer_t p_ctx, guest_pointer_t p_buffer, size_t size);
bool sys_hash_final(eret_t *ret, const cx_hash_id_t hash_id, guest_pointer_t p_ctx, guest_pointer_t p_addr);
