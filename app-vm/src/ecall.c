#include <string.h>

#include "cx.h"
#include "os.h"

#include "apdu.h"
#include "ecall.h"
#include "error.h"
#include "rv.h"
#include "stream.h"
#include "types.h"


struct cmd_send_buffer_s {
    uint8_t data[249];
    uint8_t size;
    uint32_t counter;
    uint16_t cmd;
} __attribute__((packed));

struct cmd_recv_buffer_s {
    uint32_t counter;
    uint8_t maxsize;
    uint16_t cmd;
} __attribute__((packed));

struct response_s {
    uint8_t cla;
    uint8_t ins;
    uint8_t p1;
    uint8_t p2;
    uint8_t lc;
    uint8_t data[255];
} __attribute__((packed));

static void parse_apdu(const struct response_s *response, size_t size) {
    _Static_assert(IO_APDU_BUFFER_SIZE >= sizeof(*response), "invalid IO_APDU_BUFFER_SIZE");

    if (size < OFFSET_CDATA || size - OFFSET_CDATA != response->lc) {
        fatal("invalid apdu\n");
    }
}

static void debug_write(char *msg)
{
  asm volatile (
     "movs r0, #0x04\n"
     "movs r1, %0\n"
     "svc      0xab\n"
     :: "r"(msg) : "r0", "r1"
  );
}

/*
 * Receives at most size bytes.
 *
 * Can't be interrupted by a button press for now when the exchange has started.
 * TODO: display some progress bar or something.
 */
static size_t xrecv(uint32_t addr, size_t size)
{
    size_t ret = 0;
    uint32_t counter = 0;

    while (size > 0) {
        struct response_s *response = (struct response_s *)G_io_apdu_buffer;
        size_t n;

        n = PAGE_SIZE - (addr - PAGE_START(addr));
        n = MIN(size, n);
        n = MIN(sizeof(response->data), n);

        /* retrieve buffer pointer now since it can modify G_io_apdu_buffer */
        uint8_t *buffer = get_buffer(addr, n, true);

        /* 1. send "recv" request */
        struct cmd_recv_buffer_s *cmd = (struct cmd_recv_buffer_s *)G_io_apdu_buffer;

        cmd->counter = counter;
        cmd->maxsize = n;
        cmd->cmd = (CMD_RECV_BUFFER >> 8) | ((CMD_RECV_BUFFER & 0xff) << 8);

        size_t received = io_exchange(CHANNEL_APDU, sizeof(*cmd));

        /* 2. ensure the data received fits in the buffer */

        parse_apdu(response, received);
        bool stop = (response->p1 == '\x01');

        if (response->lc > n || (response->lc != n && !stop)) {
            fatal("invalid response size\n");
        }

        n = response->lc;

        /* 3. copies data to the app buffer */
        memcpy(buffer, response->data, n);

        addr += n;
        size -= n;
        ret += n;
        counter++;

        if (stop) {
            break;
        }

        if (size == 0 && !stop) {
            fatal("invalid p1\n");
        }
    }

    return ret;
}

static void xsend(uint32_t addr, size_t size)
{
    uint32_t counter = 0;

    while (size > 0) {
        struct cmd_send_buffer_s *cmd = (struct cmd_send_buffer_s *)G_io_apdu_buffer;
        _Static_assert(IO_APDU_BUFFER_SIZE >= sizeof(*cmd), "invalid IO_APDU_BUFFER_SIZE");

        size_t n;

        n = PAGE_SIZE - (addr - PAGE_START(addr));
        n = MIN(size, n);
        n = MIN(sizeof(cmd->data), n);

        /* 0. copy the app buffer (note that it can modify G_io_apdu_buffer) */
        uint8_t *buffer;

        buffer = get_buffer(addr, n, false);
        memcpy(cmd->data, buffer, n);
        memset(cmd->data + n, '\x00', sizeof(cmd->data) - n);

        /* 1. prepare the "send" request */
        cmd->counter = counter;
        if (size - n == 0) {
            cmd->counter |= 0x80000000;
        }
        cmd->cmd = (CMD_SEND_BUFFER >> 8) | ((CMD_SEND_BUFFER & 0xff) << 8);
        cmd->size = n;

        /* 2. */
        io_exchange(CHANNEL_APDU, sizeof(*cmd));

        addr += n;
        size -= n;
        counter++;
    }
}

static void sha256sum(uint32_t data_addr, size_t size, uint32_t digest_addr)
{
    uint8_t digest[CX_SHA256_SIZE];
    cx_sha256_t ctx;

    cx_sha256_init_no_throw(&ctx);

    /* compute digest over the guest data */
    while (size > 0) {
        size_t n;
        n = PAGE_SIZE - (data_addr - PAGE_START(data_addr));
        n = MIN(size, n);

        uint8_t *buffer;
        buffer = get_buffer(data_addr, n, false);

        if (size - n != 0) {
            cx_hash_no_throw((cx_hash_t *)&ctx, 0, buffer, n, NULL, 0);
        } else {
            cx_hash_no_throw((cx_hash_t *)&ctx, CX_LAST, buffer, n, digest, sizeof(digest));
        }

        data_addr += n;
        size -= n;
    }

    /* copy digest to the guest addr */
    size = sizeof(digest);
    while (size > 0) {
        size_t n;
        n = PAGE_SIZE - (digest_addr - PAGE_START(digest_addr));
        n = MIN(size, n);

        uint8_t *buffer = get_buffer(digest_addr, n, true);
        memcpy(buffer, digest + sizeof(digest) - size, n);

        digest_addr += n;
        size -= n;
    }
}

void ecall(struct rv_cpu *cpu)
{
    uint32_t nr = cpu->regs[5];
    uint32_t ret = 0;

    switch (nr) {
    case 1:
        debug_write((char *)cpu->regs[10]);
        break;
    case 2:
        xsend(cpu->regs[10], cpu->regs[11]);
        break;
    case 3:
        ret = xrecv(cpu->regs[10], cpu->regs[11]);
        cpu->regs[10] = ret;
        break;
    case 4:
        sha256sum(cpu->regs[10], cpu->regs[11], cpu->regs[12]);
        break;
    default:
        os_sched_exit(cpu->regs[10]);
        break;
    }
}

