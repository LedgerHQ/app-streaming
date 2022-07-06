#include <string.h>

#include "bagl.h"
#include "cx.h"
#include "os.h"

#include "apdu.h"
#include "ecall.h"
#include "error.h"
#include "io.h"
#include "loading.h"
#include "rv_cpu.h"
#include "stream.h"
#include "ui.h"

struct cmd_send_buffer_s {
    uint8_t data[253];
    uint8_t size;
    uint32_t counter;
    uint16_t cmd;
} __attribute__((packed));

struct cmd_recv_buffer_s {
    uint32_t counter;
    uint16_t maxsize;
    uint16_t cmd;
} __attribute__((packed));

struct received_buffer_s {
    uint8_t stop;
    uint8_t data[259];
} __attribute__((packed));

struct cmd_exit_s {
    uint32_t code;
    uint16_t cmd;
} __attribute__((packed));

struct cmd_fatal_s {
    uint8_t msg[254];
    uint16_t cmd;
} __attribute__((packed));

/**
 * Receives at most size bytes.
 *
 * @return false on error, true otherwise
 */
bool sys_xrecv(eret_t *eret, guest_pointer_t p_buf, size_t size)
{
    uint32_t counter = 0;

    eret->size = 0;
    while (size > 0) {
        struct received_buffer_s *received_buffer = (struct received_buffer_s *)G_io_apdu_buffer;
        _Static_assert(IO_APDU_BUFFER_SIZE >= sizeof(*received_buffer), "invalid IO_APDU_BUFFER_SIZE");

        size_t n = BUFFER_MIN_SIZE(p_buf.addr, size);
        n = MIN(sizeof(received_buffer->data), n);

        /* 0. retrieve buffer pointer now since it can modify G_io_apdu_buffer */
        uint8_t *buffer = get_buffer(p_buf.addr, n, true);
        if (buffer == NULL) {
            return false;
        }

        /* 1. send "recv" request */
        struct cmd_recv_buffer_s *cmd = (struct cmd_recv_buffer_s *)G_io_apdu_buffer;
        _Static_assert(IO_APDU_BUFFER_SIZE >= sizeof(*cmd), "invalid IO_APDU_BUFFER_SIZE");

        cmd->counter = counter;
        cmd->maxsize = n;
        cmd->cmd = (CMD_RECV_BUFFER >> 8) | ((CMD_RECV_BUFFER & 0xff) << 8);

        size_t received = io_exchange(CHANNEL_APDU, sizeof(*cmd));

        /* 2. ensure that data received fits in the buffer */
        bool stop = (received_buffer->stop != '\0');

        if ((received - 1) > n || ((received - 1) != n && !stop)) {
            err("invalid data size\n");
            return false;
        }

        n = received - 1;

        /* 3. copies data to the app buffer */
        memcpy(buffer, received_buffer->data, n);

        p_buf.addr += n;
        size -= n;
        eret->size += n;
        counter++;

        if (stop) {
            break;
        }

        if (size == 0 && !stop) {
            err("missing stop byte\n");
            return false;
        }
    }

    return true;
}

/**
 * @return false on error, true otherwise
 */
bool sys_xsend(guest_pointer_t p_buf, size_t size)
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
            return false;
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

    return true;
}

bool sys_fatal(guest_pointer_t p_msg, size_t size)
{
    struct cmd_fatal_s *cmd = (struct cmd_fatal_s *)G_io_apdu_buffer;

    _Static_assert(IO_APDU_BUFFER_SIZE >= sizeof(*cmd) + 1, "invalid IO_APDU_BUFFER_SIZE");

    if (size > sizeof(cmd->msg) - 1) {
        size = sizeof(cmd->msg) - 1;
    }

    if (!copy_guest_buffer(p_msg, cmd->msg, size))  {
        return false;
    }

    memset(cmd->msg + size, '\x00', sizeof(cmd->msg) - size);

    cmd->cmd = (CMD_FATAL >> 8) | ((CMD_FATAL & 0xff) << 8);

    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, sizeof(*cmd));

    return true;
}

void sys_exit(unsigned int code)
{
    struct cmd_exit_s *cmd = (struct cmd_exit_s *)G_io_apdu_buffer;

    cmd->code = code;
    cmd->cmd = (CMD_EXIT >> 8) | ((CMD_EXIT & 0xff) << 8);

    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, sizeof(*cmd));
}

#if defined(TARGET_NANOX) || defined(TARGET_NANOS2)
void sys_ux_rectangle(unsigned int color, unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{
    bagl_hal_draw_rect(color, x, y, width, height);
}

void sys_screen_update(void)
{
    screen_update();
}

bool copy_guest_buffer(guest_pointer_t p_src, void *buf, size_t size)
{
    uint8_t *dst = buf;

    while (size > 0) {
        const size_t n = BUFFER_MIN_SIZE(p_src.addr, size);
        const uint8_t *buffer = get_buffer(p_src.addr, n, false);
        if (buffer == NULL) {
            return false;
        }

        memcpy(dst, buffer, n);

        p_src.addr += n;
        dst += n;
        size -= n;
    }

    return true;
}

bool copy_host_buffer(guest_pointer_t p_dst, void *buf, size_t size)
{
    uint8_t *src = buf;

    while (size > 0) {
        const size_t n = BUFFER_MIN_SIZE(p_dst.addr, size);
        uint8_t *buffer = get_buffer(p_dst.addr, n, true);
        if (buffer == NULL) {
            return false;
        }

        memcpy(buffer, src, n);

        p_dst.addr += n;
        src += n;
        size -= n;
    }

    return true;
}

bool sys_ux_bitmap(int x, int y, unsigned int width, unsigned int height, /*unsigned int color_count,*/ guest_pointer_t p_colors, unsigned int bit_per_pixel, guest_pointer_t p_bitmap, unsigned int bitmap_length_bits)
{
    unsigned int colors[2];
    uint8_t bitmap[512];

    size_t bitmap_length = bitmap_length_bits / 8;
    if (bitmap_length_bits % 8 != 0) {
        bitmap_length += 1;
    }

    if (!copy_guest_buffer(p_colors, colors, 2 * sizeof(unsigned int))) {
        return false;
    }

    if (!copy_guest_buffer(p_bitmap, bitmap, bitmap_length)) {
        return false;
    }

    bagl_hal_draw_bitmap_within_rect(x, y, width, height, /*color_count, */2, colors, bit_per_pixel, bitmap, bitmap_length_bits);

    return true;
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

bool sys_bagl_draw_with_context(guest_pointer_t p_component, guest_pointer_t p_context, size_t context_length, int context_encoding)
{
    packed_bagl_component_t packed_component;
    bagl_component_t component;
    uint8_t context_buf[64];
    void *context;

    if (context_length > sizeof(context_buf)) {
        context_length = sizeof(context_buf);
    }

    if (!copy_guest_buffer(p_component, &packed_component, sizeof(packed_component))) {
        return false;
    }

    if (p_context.addr != 0) {
        if (!copy_guest_buffer(p_context, &context_buf, context_length))  {
            return false;
        }
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

    return true;
}

void sys_ux_idle(void)
{
    ui_app_idle();
}

bool sys_memset(eret_t *eret, guest_pointer_t p_s, int c, size_t size)
{
    eret->addr = p_s.addr;

    while (size > 0) {
        const size_t n = BUFFER_MIN_SIZE(p_s.addr, size);
        uint8_t *buffer = get_buffer(p_s.addr, n, true);
        if (buffer == NULL) {
            return false;
        }

        memset(buffer, c, n);

        p_s.addr += n;
        size -= n;
    }

    return true;
}

bool sys_memcpy(eret_t *eret, guest_pointer_t p_dst, guest_pointer_t p_src, size_t size)
{
    eret->addr = p_dst.addr;

    while (size > 0) {
        const size_t a = BUFFER_MIN_SIZE(p_dst.addr, size);
        const size_t n = BUFFER_MIN_SIZE(p_src.addr, a);
        const uint8_t *buffer_src = get_buffer(p_src.addr, n, false);
        if (buffer_src == NULL) {
            return false;
        }

        /* a temporary buffer is required because get_buffer() might unlikely
         * return the same page if p_dst.addr or p_src.addr isn't in the
         * cache. */
        uint8_t tmp[PAGE_SIZE];
        memcpy(tmp, buffer_src, n);

        uint8_t *buffer_dst = get_buffer(p_dst.addr, n, true);
        if (buffer_dst == NULL) {
            return false;
        }

        memcpy(buffer_dst, tmp, n);

        p_dst.addr += n;
        p_src.addr += n;
        size -= n;
    }

    return true;
}

bool sys_strlen(eret_t *eret, guest_pointer_t p_s)
{
    eret->size = 0;

    while (true) {
        const size_t n = BUFFER_MAX_SIZE(p_s.addr);
        const char *buffer = (char *)get_buffer(p_s.addr, n, false);
        if (buffer == NULL) {
            return false;
        }

        const size_t tmp_size = strnlen(buffer, n);

        p_s.addr += n;
        eret->size += tmp_size;

        if (tmp_size < n) {
            break;
        }
    }

    return true;
}

bool sys_strnlen(eret_t *eret, guest_pointer_t p_s, size_t maxlen)
{
    eret->size = 0;

    while (maxlen > 0) {
        const size_t n = BUFFER_MIN_SIZE(p_s.addr, maxlen);
        const char *buffer = (char *)get_buffer(p_s.addr, n, false);
        if (buffer == NULL) {
            return false;
        }

        size_t tmp_size = strnlen(buffer, n);

        p_s.addr += n;
        maxlen -= n;
        eret->size += tmp_size;

        if (tmp_size < n) {
            break;
        }
    }

    return true;
}
