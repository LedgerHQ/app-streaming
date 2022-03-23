#pragma once

#define ECALL_FATAL            1
#define ECALL_XSEND            2
#define ECALL_XRECV            3
#define ECALL_SHA256SUM        4
#define ECALL_EXIT             5
#define ECALL_UX_RECTANGLE     6
#define ECALL_SCREEN_UPDATE    7
#define ECALL_BAGL_DRAW_BITMAP 8
#define ECALL_WAIT_BUTTON      9
#define ECALL_BAGL_DRAW        10
#define ECALL_LOADING_START    11
#define ECALL_LOADING_STOP     12
#define ECALL_UX_IDLE          13
#define ECALL_MEMSET           14
#define ECALL_MEMCPY           15
#define ECALL_STRLEN           16

#define ECALL_DERIVE_NODE_BIP32     100
#define ECALL_CX_ECFP_GENERATE_PAIR 101
#define ECALL_CX_ECFP_GET_PUBKEY    102
#define ECALL_CX_SHA3_256           103
#define ECALL_ECDSA_SIGN            104
#define ECALL_MULT                  105
#define ECALL_MULTM                 106
#define ECALL_TOSTRING256           107
#define ECALL_HASH_UPDATE           108
#define ECALL_HASH_FINAL            109
