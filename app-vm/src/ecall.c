#include <string.h>

#include "bagl.h"
#include "cx.h"
#include "os.h"
#include "os_io_seproxyhal.h"

#include "apdu.h"
#include "ecall.h"
#include "error.h"
#include "loading.h"
#include "rv.h"
#include "stream.h"
#include "types.h"

#include "sdk/ecall-nr.h"

int saved_apdu_state;


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

struct cmd_fatal_s {
    uint8_t msg[254];
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

#if false
        if (ret == 0 && G_io_app.apdu_state == 0xff) {
            // restore G_io_app
            G_io_app.apdu_state = saved_apdu_state;
            G_io_app.apdu_length = 0;
            G_io_app.ms = 0;
            err("ok button pressed\n");
            /* button */
            return 0xdeadbe00 | 2;
        } else {
            fatal("wtf\n");
        }
#endif

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

static void sys_fatal(uint32_t msg_addr)
{
    struct cmd_fatal_s *cmd = (struct cmd_fatal_s *)G_io_apdu_buffer;

    _Static_assert(IO_APDU_BUFFER_SIZE >= sizeof(*cmd) + 1, "invalid IO_APDU_BUFFER_SIZE");

    /* copy error message, which might be on 2 contiguous pages */
    size_t max_size = sizeof(cmd->msg);

    uint8_t *p = cmd->msg;
    size_t n = PAGE_SIZE - (msg_addr - PAGE_START(msg_addr));
    if (n > max_size) {
        n = max_size;
    }
    copy_guest_buffer(msg_addr, p, n);

    uint8_t *q = memchr(p, '\x00', n);
    if (q == NULL) {
        msg_addr += n;
        p += n;
        max_size -= n;
        copy_guest_buffer(msg_addr, p, max_size);
        q = memchr(p, '\x00', n);
    }

    if (q != NULL) {
        memset(q, '\x00', sizeof(cmd->msg) - (q - p));
    }

    cmd->cmd = (CMD_FATAL >> 8) | ((CMD_FATAL & 0xff) << 8);

    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, sizeof(*cmd));
}

void sys_exit(uint32_t code)
{
    struct cmd_exit_s *cmd = (struct cmd_exit_s *)G_io_apdu_buffer;

    cmd->code = code;
    cmd->cmd = (CMD_EXIT >> 8) | ((CMD_EXIT & 0xff) << 8);

    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, sizeof(*cmd));
}

#ifdef TARGET_NANOX
static void sys_ux_rectangle(uint32_t color, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    bagl_hal_draw_rect(color, x, y, width, height);
}

static void sys_screen_update(void)
{
    screen_update();
}

void copy_guest_buffer(uint32_t addr, void *buf, size_t size)
{
    uint8_t *dst = buf;

    while (size > 0) {
        size_t n;
        n = PAGE_SIZE - (addr - PAGE_START(addr));
        n = MIN(size, n);

        uint8_t *buffer;
        buffer = get_buffer(addr, n, false);
        memcpy(dst, buffer, n);

        addr += n;
        dst += n;
        size -= n;
    }
}

void copy_host_buffer(uint32_t addr, void *buf, size_t size)
{
    uint8_t *dst = buf;

    while (size > 0) {
        size_t n;
        n = PAGE_SIZE - (addr - PAGE_START(addr));
        n = MIN(size, n);

        uint8_t *buffer;
        buffer = get_buffer(addr, n, true);
        memcpy(buffer, dst, n);

        addr += n;
        dst += n;
        size -= n;
    }
}

static void sys_ux_bitmap(int x, int y, unsigned int width, unsigned int height, /*unsigned int color_count,*/ uint32_t colors_addr, unsigned int bit_per_pixel, uint32_t bitmap_addr, unsigned int bitmap_length_bits)
{
    unsigned int colors[2];
    uint8_t bitmap[512];

    size_t bitmap_length = bitmap_length_bits / 8;
    if (bitmap_length_bits % 8 != 0) {
        bitmap_length += 1;
    }

    copy_guest_buffer(colors_addr, colors, 2 * sizeof(unsigned int));
    copy_guest_buffer(bitmap_addr, bitmap, bitmap_length);

    bagl_hal_draw_bitmap_within_rect(x, y, width, height, /*color_count, */2, colors, bit_per_pixel, bitmap, bitmap_length_bits);
}
#endif

#define BUTTON_LEFT             (1 << 0)
#define BUTTON_RIGHT            (1 << 1)

#define BUTTON_EVT_FAST         0x40000000
#define BUTTON_EVT_RELEASED     0x80000000

#define BUTTON_FAST_THRESHOLD_CS 8 //x100MS
#define BUTTON_FAST_ACTION_CS    3 //x100MS

static unsigned int g_button_mask;
static unsigned int g_button_same_mask_counter;

int button_pressed;
bool wait_for_button;

static int to_button_mask(unsigned int new_button_mask)
{
    unsigned int button_mask;
    unsigned int button_same_mask_counter;

    if (new_button_mask == g_button_mask) {
        g_button_same_mask_counter++;
    }

    button_mask = g_button_mask | new_button_mask;
    button_same_mask_counter = g_button_same_mask_counter;

    if (new_button_mask == 0) {
        g_button_mask = 0;
        g_button_same_mask_counter = 0;
        button_mask |= BUTTON_EVT_RELEASED;
    }
    else {
        g_button_mask = button_mask;
    }

    if (new_button_mask != g_button_mask) {
        g_button_same_mask_counter = 0;
    }

    if (button_same_mask_counter >= BUTTON_FAST_THRESHOLD_CS) {
        if ((button_same_mask_counter % BUTTON_FAST_ACTION_CS) == 0) {
            button_mask |= BUTTON_EVT_FAST;
        }

        button_mask &= ~BUTTON_EVT_RELEASED;
    }

    if (!(button_mask & BUTTON_EVT_RELEASED)) {
        return 0;
    }

    return (button_mask & (BUTTON_LEFT | BUTTON_RIGHT));
}

/* equivalent to io_exchange with IO_ASYNCH_REPLY */
static unsigned short io_exchange_asynch_reply(void)
{
  G_io_app.apdu_length = 0;

  button_pressed = 0;
  wait_for_button = true;

  do {
      io_seproxyhal_general_status();

      size_t rx_len = io_seproxyhal_spi_recv(G_io_seproxyhal_spi_buffer, sizeof(G_io_seproxyhal_spi_buffer), 0);
      if (rx_len < 3 || rx_len != U2(G_io_seproxyhal_spi_buffer[1],G_io_seproxyhal_spi_buffer[2])+3U) {
          G_io_app.apdu_state = APDU_IDLE;
          G_io_app.apdu_length = 0;
          continue;
      }

      io_seproxyhal_handle_event();
  } while (button_pressed == 0);

  wait_for_button = false;

  return button_pressed - 1;
}

static int sys_wait_button(void)
{
    int button_mask;

    do {
        int button = io_exchange_asynch_reply();
        button_mask = to_button_mask(button);
    } while (button_mask == 0);

    return button_mask;
}

typedef struct {
  unsigned char type;
  unsigned char userid;
  short x;
  short y;
  unsigned short width;
  unsigned short height;
  unsigned char stroke;
  unsigned char radius;
  unsigned char fill;
  unsigned int fgcolor;
  unsigned int bgcolor;
  unsigned short font_id;
  unsigned char icon_id;
} __attribute__((packed)) packed_bagl_component_t;

static void sys_bagl_draw_with_context(uint32_t component_addr, uint32_t context_addr, size_t context_length, int context_encoding)
{
    packed_bagl_component_t packed_component;
    bagl_component_t component;
    uint8_t context_buf[64];
    void *context;

    if (context_length > sizeof(context_buf)) {
        context_length = sizeof(context_buf);
    }

    copy_guest_buffer(component_addr, &packed_component, sizeof(packed_component));
    if (context_addr != 0) {
        copy_guest_buffer(context_addr, &context_buf, context_length);
        context = &context_buf;
    } else {
        context = NULL;
    }

    component.type = packed_component.type;
    component.userid = packed_component.userid;
    component.x = packed_component.x;
    component.y = packed_component.y;
    component.width = packed_component.width;
    component.height = packed_component.height;
    component.stroke = packed_component.stroke;
    component.radius = packed_component.radius;
    component.fill = packed_component.fill;
    component.fgcolor = packed_component.fgcolor;
    component.bgcolor = packed_component.bgcolor;
    component.font_id = packed_component.font_id;
    component.icon_id = packed_component.icon_id;

    bagl_draw_with_context(&component, context, context_length, context_encoding);
}

/*
 * Return true if the ecall either exit() or unsupported, false otherwise.
 */
bool ecall(struct rv_cpu *cpu)
{
    uint32_t nr = cpu->regs[5];
    bool stop = false;

    switch (nr) {
    case ECALL_FATAL:
        sys_fatal(cpu->regs[10]);
        stop = true;
        break;
    case ECALL_XSEND:
        xsend(cpu->regs[10], cpu->regs[11]);
        break;
    case ECALL_XRECV:
        cpu->regs[10] = xrecv(cpu->regs[10], cpu->regs[11]);
        break;
    case ECALL_SHA256SUM:
        sha256sum(cpu->regs[10], cpu->regs[11], cpu->regs[12]);
        break;
    case ECALL_EXIT:
        sys_exit(cpu->regs[10]);
        stop = true;
        break;
#ifdef TARGET_NANOX
    case ECALL_UX_RECTANGLE:
        sys_ux_rectangle(cpu->regs[10], cpu->regs[11], cpu->regs[12], cpu->regs[13], cpu->regs[14]);
        break;
    case ECALL_SCREEN_UPDATE:
        sys_screen_update();
        break;
    case ECALL_BAGL_DRAW_BITMAP:
        sys_ux_bitmap(cpu->regs[10], cpu->regs[11], cpu->regs[12], cpu->regs[13], cpu->regs[14], cpu->regs[15], cpu->regs[16], cpu->regs[17]);
        break;
#endif
    case ECALL_WAIT_BUTTON:
        cpu->regs[10] = sys_wait_button();
        break;
    case ECALL_BAGL_DRAW:
        sys_bagl_draw_with_context(cpu->regs[10], cpu->regs[11], cpu->regs[12], cpu->regs[13]);
        break;
    case ECALL_LOADING_START:
        app_loading_start();
        break;
    case ECALL_LOADING_STOP:
        cpu->regs[10] = app_loading_stop();
        break;
    default:
        stop = ecall_bolos(cpu, nr);
        break;
    }

    return stop;
}
