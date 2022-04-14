#pragma once

bool os_perso_derive_node_bip32_nt(cx_curve_t curve, const uint32_t *path, size_t pathLength, uint8_t *privateKey, uint8_t *chain);
