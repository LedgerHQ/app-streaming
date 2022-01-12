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
    uint16_t maxsize;
    uint16_t cmd;
} __attribute__((packed));

struct cmd_exit_s {
    uint32_t code;
    uint16_t cmd;
} __attribute__((packed));

struct response_s {
    uint8_t cla;
    uint8_t ins;
    uint8_t p1;
    uint8_t p2;
    uint8_t lc;
    uint8_t data[PAGE_SIZE - 1];
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
        /* an additional byte is stored in p2 to allow entire pages to be
         * transmitted, hence + 1 */
        _Static_assert(IO_APDU_BUFFER_SIZE >= sizeof(response->data) + 1, "invalid IO_APDU_BUFFER_SIZE");

        size_t n;
        n = PAGE_SIZE - (addr - PAGE_START(addr));
        n = MIN(size, n);

        /* 0. retrieve buffer pointer now since it can modify G_io_apdu_buffer */
        uint8_t *buffer = get_buffer(addr, n, true);

        /* 1. send "recv" request */
        struct cmd_recv_buffer_s *cmd = (struct cmd_recv_buffer_s *)G_io_apdu_buffer;

        cmd->counter = counter;
        cmd->maxsize = n;
        cmd->cmd = (CMD_RECV_BUFFER >> 8) | ((CMD_RECV_BUFFER & 0xff) << 8);

        size_t received = io_exchange(CHANNEL_APDU, sizeof(*cmd));

        /* 2. ensure that data received fits in the buffer */

        parse_apdu(response, received);
        bool stop = (response->p1 == '\x01');

        if ((response->lc + 1) > n || ((response->lc + 1) != n && !stop)) {
            fatal("invalid response size\n");
        }

        n = response->lc + 1;

        /* 3. copies data to the app buffer (the first byte is in p2) */
        buffer[0] = response->p2;
        memcpy(buffer + 1, response->data, n - 1);

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

static void sys_exit(uint32_t code)
{
    struct cmd_exit_s *cmd = (struct cmd_exit_s *)G_io_apdu_buffer;

    cmd->code = code;
    cmd->cmd = (CMD_EXIT >> 8) | ((CMD_EXIT & 0xff) << 8);

    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, sizeof(*cmd));
}

static void sys_ux_rectangle(uint32_t color, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
#ifdef TARGET_NANOX
    bagl_hal_draw_rect(color, x, y, width, height);
#endif
}

/*
 * Return true if the ecall either exit() or unsupported, false otherwise.
 */
bool ecall(struct rv_cpu *cpu)
{
    uint32_t nr = cpu->regs[5];
    bool stop = false;

    switch (nr) {
    case 1:
        debug_write((char *)cpu->regs[10]);
        break;
    case 2:
        xsend(cpu->regs[10], cpu->regs[11]);
        break;
    case 3:
        cpu->regs[10] = xrecv(cpu->regs[10], cpu->regs[11]);
        break;
    case 4:
        sha256sum(cpu->regs[10], cpu->regs[11], cpu->regs[12]);
        break;
    case 5:
        sys_exit(cpu->regs[10]);
        stop = true;
        break;
    case 6:
        sys_ux_rectangle(cpu->regs[10], cpu->regs[11], cpu->regs[12], cpu->regs[13], cpu->regs[14]);
        break;
    default:
        sys_exit(0xdeaddead);
        stop = true;
        break;
    }

    return stop;
}

