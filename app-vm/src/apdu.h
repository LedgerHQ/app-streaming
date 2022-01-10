#pragma once

#define OFFSET_CLA 0
#define OFFSET_INS 1
#define OFFSET_P1 2
#define OFFSET_P2 3
#define OFFSET_LC 4
#define OFFSET_CDATA 5

enum cmd_stream_e {
    CMD_REQUEST_PAGE = 0x6101,
    CMD_REQUEST_HMAC = 0x6102,
    CMD_REQUEST_PROOF = 0x6103,
    CMD_COMMIT_PAGE = 0x6201,
    CMD_COMMIT_HMAC = 0x6202,
    CMD_SEND_BUFFER = 0x6301,
    CMD_RECV_BUFFER = 0x6401,
};

