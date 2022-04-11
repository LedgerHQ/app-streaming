#include <string.h>

#include "bagl.h"
#include "cx.h"
#include "os.h"

#include "apdu.h"
#include "ecall.h"
#include "error.h"
#include "loading.h"
#include "rv.h"
#include "stream.h"
#include "types.h"
#include "ui.h"

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

/*
 * Receives at most size bytes.
 *
 * Can't be interrupted by a button press for now when the exchange has started.
 * TODO: display some progress bar or something.
 */
size_t sys_xrecv(guest_pointer_t p_buf, size_t size)
{
    size_t ret = 0;
    uint32_t counter = 0;

    while (size > 0) {
        struct apdu_s *apdu = (struct apdu_s *)G_io_apdu_buffer;
        /* an additional byte is stored in p2 to allow entire pages to be
         * transmitted, hence + 1 */
        _Static_assert(IO_APDU_BUFFER_SIZE >= sizeof(apdu->data) + 1, "invalid IO_APDU_BUFFER_SIZE");

        size_t n = BUFFER_MIN_SIZE(p_buf.addr, size);

        /* 0. retrieve buffer pointer now since it can modify G_io_apdu_buffer */
        uint8_t *buffer = get_buffer(p_buf.addr, n, true);
        if (buffer == NULL) {
            fatal("get_buffer failed\n");
        }

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

        apdu = parse_apdu(received);
        if (apdu == NULL) {
            fatal("invalid APDU\n");
        }
        bool stop = (apdu->p1 == '\x01');

        if ((apdu->lc + 1) > n || ((apdu->lc + 1) != n && !stop)) {
            fatal("invalid apdu size\n");
        }

        n = apdu->lc + 1;

        /* 3. copies data to the app buffer (the first byte is in p2) */
        buffer[0] = apdu->p2;
        memcpy(buffer + 1, apdu->data, n - 1);

        p_buf.addr += n;
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

void sys_xsend(guest_pointer_t p_buf, size_t size)
{
    uint32_t counter = 0;

    while (size > 0) {
        struct cmd_send_buffer_s *cmd = (struct cmd_send_buffer_s *)G_io_apdu_buffer;
        _Static_assert(IO_APDU_BUFFER_SIZE >= sizeof(*cmd), "invalid IO_APDU_BUFFER_SIZE");

        size_t n = BUFFER_MIN_SIZE(p_buf.addr, size);
        n = MIN(sizeof(cmd->data), n);

        /* 0. copy the app buffer (note that it can modify G_io_apdu_buffer) */
        const uint8_t *buffer = get_buffer(p_buf.addr, n, false);
        if (buffer == NULL) {
            fatal("get_buffer failed\n");
        }

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

        p_buf.addr += n;
        size -= n;
        counter++;
    }
}

void sys_fatal(guest_pointer_t p_msg)
{
    struct cmd_fatal_s *cmd = (struct cmd_fatal_s *)G_io_apdu_buffer;

    _Static_assert(IO_APDU_BUFFER_SIZE >= sizeof(*cmd) + 1, "invalid IO_APDU_BUFFER_SIZE");

    /* copy error message, which might be on 2 contiguous pages */
    size_t max_size = sizeof(cmd->msg);

    uint8_t *p = cmd->msg;
    size_t n = BUFFER_MAX_SIZE(p_msg.addr);
    if (n > max_size) {
        n = max_size;
    }
    copy_guest_buffer(p_msg, p, n);

    uint8_t *q = memchr(p, '\x00', n);
    if (q == NULL) {
        p_msg.addr += n;
        p += n;
        max_size -= n;
        copy_guest_buffer(p_msg, p, max_size);
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
void sys_ux_rectangle(uint32_t color, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    bagl_hal_draw_rect(color, x, y, width, height);
}

void sys_screen_update(void)
{
    screen_update();
}

void copy_guest_buffer(guest_pointer_t p_src, void *buf, size_t size)
{
    uint8_t *dst = buf;

    while (size > 0) {
        const size_t n = BUFFER_MIN_SIZE(p_src.addr, size);
        const uint8_t *buffer = get_buffer(p_src.addr, n, false);
        if (buffer == NULL) {
            fatal("get_buffer failed\n");
        }

        memcpy(dst, buffer, n);

        p_src.addr += n;
        dst += n;
        size -= n;
    }
}

void copy_host_buffer(guest_pointer_t p_dst, void *buf, size_t size)
{
    uint8_t *src = buf;

    while (size > 0) {
        const size_t n = BUFFER_MIN_SIZE(p_dst.addr, size);
        uint8_t *buffer = get_buffer(p_dst.addr, n, true);
        if (buffer == NULL) {
            fatal("get_buffer failed\n");
        }

        memcpy(buffer, src, n);

        p_dst.addr += n;
        src += n;
        size -= n;
    }
}

void sys_ux_bitmap(int x, int y, unsigned int width, unsigned int height, /*unsigned int color_count,*/ guest_pointer_t p_colors, unsigned int bit_per_pixel, guest_pointer_t p_bitmap, unsigned int bitmap_length_bits)
{
    unsigned int colors[2];
    uint8_t bitmap[512];

    size_t bitmap_length = bitmap_length_bits / 8;
    if (bitmap_length_bits % 8 != 0) {
        bitmap_length += 1;
    }

    copy_guest_buffer(p_colors, colors, 2 * sizeof(unsigned int));
    copy_guest_buffer(p_bitmap, bitmap, bitmap_length);

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

int sys_wait_button(void)
{
    int button_mask;

    do {
        int button = io_exchange_asynch_reply();
        button_mask = to_button_mask(button);
    } while (button_mask == 0);

    return button_mask;
}

void sys_bagl_draw_with_context(guest_pointer_t p_component, guest_pointer_t p_context, size_t context_length, int context_encoding)
{
    packed_bagl_component_t packed_component;
    bagl_component_t component;
    uint8_t context_buf[64];
    void *context;

    if (context_length > sizeof(context_buf)) {
        context_length = sizeof(context_buf);
    }

    copy_guest_buffer(p_component, &packed_component, sizeof(packed_component));
    if (p_context.addr != 0) {
        copy_guest_buffer(p_context, &context_buf, context_length);
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

void sys_ux_idle(void)
{
    ui_app_idle();
}

uint32_t sys_memset(guest_pointer_t p_s, int c, size_t size)
{
    const uint32_t s_addr = p_s.addr;

    while (size > 0) {
        const size_t n = BUFFER_MIN_SIZE(p_s.addr, size);
        uint8_t *buffer = get_buffer(p_s.addr, n, true);
        if (buffer == NULL) {
            fatal("get_buffer failed\n");
        }

        memset(buffer, c, n);

        p_s.addr += n;
        size -= n;
    }

    return s_addr;
}

uint32_t sys_memcpy(guest_pointer_t p_dst, guest_pointer_t p_src, size_t size)
{
    const uint32_t dst_addr = p_dst.addr;

    while (size > 0) {
        const size_t a = BUFFER_MIN_SIZE(p_dst.addr, size);
        const size_t n = BUFFER_MIN_SIZE(p_src.addr, a);
        const uint8_t *buffer_src = get_buffer(p_src.addr, n, false);
        if (buffer_src == NULL) {
            fatal("get_buffer failed\n");
        }

        /* a temporary buffer is required because get_buffer() might unlikely
         * return the same page if p_dst.addr or p_src.addr isn't in the
         * cache. */
        uint8_t tmp[PAGE_SIZE];
        memcpy(tmp, buffer_src, n);

        uint8_t *buffer_dst = get_buffer(p_dst.addr, n, true);
        if (buffer_dst == NULL) {
            fatal("get_buffer failed\n");
        }

        memcpy(buffer_dst, tmp, n);

        p_dst.addr += n;
        p_src.addr += n;
        size -= n;
    }

    return dst_addr;
}

size_t sys_strlen(guest_pointer_t p_s)
{
    size_t size = 0;

    while (true) {
        const size_t n = BUFFER_MAX_SIZE(p_s.addr);
        const char *buffer = (char *)get_buffer(p_s.addr, n, false);
        if (buffer == NULL) {
            fatal("get_buffer failed\n");
        }

        const size_t tmp_size = strnlen(buffer, n);

        p_s.addr += n;
        size += tmp_size;

        if (tmp_size < n) {
            break;
        }
    }

    return size;
}

size_t sys_strnlen(guest_pointer_t p_s, size_t maxlen)
{
    size_t size = 0;

    while (maxlen > 0) {
        const size_t n = BUFFER_MIN_SIZE(p_s.addr, maxlen);
        const char *buffer = (char *)get_buffer(p_s.addr, n, false);
        if (buffer == NULL) {
            fatal("get_buffer failed\n");
        }

        size_t tmp_size = strnlen(buffer, n);

        p_s.addr += n;
        maxlen -= n;
        size += tmp_size;

        if (tmp_size < n) {
            break;
        }
    }

    return size;
}
