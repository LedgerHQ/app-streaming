#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "sdk/api/ecall-params.h"

void sys_sha256sum(uint32_t data_addr, size_t size, uint32_t digest_addr);
void sys_sha3_256(uint32_t buffer_addr, size_t size, const uint32_t digest_addr);
bool sys_hash_update(const cx_hash_id_t hash_id, uint32_t ctx_addr, uint32_t buffer_addr, size_t size);
bool sys_hash_final(const cx_hash_id_t hash_id, uint32_t ctx_addr, uint32_t digest_addr);
