#include <stdint.h>
#include <string.h>

#include "os_io_seproxyhal.h"

#include "rv.h"
#include "stream.h"

#define PAGE_SIZE 256
#define PAGE_MASK ~(PAGE_SIZE-1)

#define PAGE_START(addr) ((addr) & PAGE_MASK)

enum page_prot_e {
    PAGE_PROT_RO,
    PAGE_PROT_RW,
};

enum cmd_stream_e {
    CMD_REQUEST_PAGE = 0x6101,
    CMD_COMMIT_PAGE = 0x6102,
};

enum section_e {
    SECTION_CODE,
    SECTION_STACK,
    SECTION_DATA,
    NUM_SECTIONS,
};

struct page_s {
    uint32_t addr;
    uint8_t data[PAGE_SIZE];
};

struct section {
    uint32_t start;
    uint32_t end;
};

struct app_s {
    struct rv_cpu cpu;

    struct section sections[NUM_SECTIONS];

    struct page_s code;
    struct page_s stack;
};

struct cmd_request_page_s {
    uint32_t addr;
    uint16_t cmd;
};

struct cmd_commit_page_s {
    uint32_t addr;
    uint16_t cmd;
};

static struct app_s app;

void err(char *msg)
{
    asm volatile (
     "movs r0, #0x04\n"
     "movs r1, %0\n"
     "svc      0xab\n"
     :: "r"(msg) : "r0", "r1"
    );
}

void fatal(char *msg)
{
    err(msg);
    os_sched_exit(7);
}

void stream_request_page(uint32_t addr, uint8_t *data)
{
    struct cmd_request_page_s *cmd = (struct cmd_request_page_s *)G_io_apdu_buffer;
    size_t size;

    //_Static_assert(IO_APDU_BUFFER_SIZE == 255 + 5, "lol");

    cmd->addr = addr;
    cmd->cmd = (CMD_REQUEST_PAGE >> 8) | ((CMD_REQUEST_PAGE & 0xff) << 8);

    size = io_exchange(CHANNEL_APDU, sizeof(*cmd)-2);
    if (size != PAGE_SIZE) {
    }

    memcpy(data, G_io_apdu_buffer, PAGE_SIZE);

    /* TODO: check page merkle hash */
}

void stream_commit_page(uint32_t addr, uint8_t *data)
{
    /* TODO: encrypt page */

    struct cmd_commit_page_s *cmd = (struct cmd_commit_page_s *)G_io_apdu_buffer;
    size_t size;

    //_Static_assert(IO_APDU_BUFFER_SIZE == 255 + 5, "lol");

    cmd->addr = addr;
    cmd->cmd = (CMD_COMMIT_PAGE >> 8) | ((CMD_COMMIT_PAGE & 0xff) << 8);

    size = io_exchange(CHANNEL_APDU, sizeof(*cmd)-2);
    if (size != 1) {
    }

    memcpy(G_io_apdu_buffer, data, PAGE_SIZE);
    size = io_exchange(CHANNEL_APDU, PAGE_SIZE);
    if (size != 1) {
    }
}

void stream_init_app(uint8_t *buffer)
{
    memset(&app, 0, sizeof(app));

    /* XXX */
    app.sections[SECTION_CODE].start = 0x10000;
    app.sections[SECTION_CODE].end = 0x20000;
    app.sections[SECTION_STACK].start = 0x70000000;
    app.sections[SECTION_STACK].end = 0x80000000;
    app.sections[SECTION_DATA].start = 0x0;
    app.sections[SECTION_DATA].end = 0x0;

    app.cpu.pc = *(uint32_t *)&buffer[5+0];
    app.cpu.regs[2] = *(uint32_t *)&buffer[5+4] - 4; // sp

    app.code.addr = 0;
    app.stack.addr = PAGE_START(app.cpu.regs[2]);
}

static bool in_section(enum section_e section, uint32_t addr)
{
    return addr >= app.sections[section].start && addr < app.sections[section].end;
}

static void u32hex(uint32_t n, char *buf)
{
    char hex[16] = "0123456789abcdef";
    size_t i;

    for (i = 0; i < 8; i++) {
        buf[i*2] = hex[(n >> ((24 - i * 8) + 4)) & 0xf];
        buf[i*2+1] = hex[(n >> (24 - i * 8)) & 0xf];
    }
}

static struct page_s *get_page(uint32_t addr, enum page_prot_e page_prot)
{
    struct page_s *page;

    addr = PAGE_START(addr);

    if (in_section(SECTION_CODE, addr)) {
        page = &app.code;
        if (page_prot != PAGE_PROT_RO) {
            fatal("write access to code page\n");
        }
    } else if (in_section(SECTION_STACK, addr)) {
        page = &app.stack;
    } else {
        char buf[19];

        memcpy(buf, "reg: ", 4);
        u32hex(addr, &buf[4]);
        buf[12] = '\n';
        buf[13] = '\x00';
        err(buf);

        memcpy(buf, "gp: ", 4);
        u32hex(app.cpu.regs[3], &buf[4]);
        buf[12] = '\n';
        buf[13] = '\x00';
        err(buf);

        fatal("invalid addr (no section found)\n");
        page = NULL;
    }

    if (addr != page->addr) {
        if (page_prot == PAGE_PROT_RW) {
            stream_commit_page(page->addr, page->data);
        }
        stream_request_page(addr, page->data);
        page->addr = addr;
    }

    return page;
}

static bool same_page(uint32_t addr1, uint32_t addr2)
{
    return (addr1 & PAGE_MASK) == (addr2 & PAGE_MASK);
}

static void check_alignment(uint32_t addr, size_t size)
{
    /*switch (size) {
    case 4:
        if (addr & 3) {
            fatal("invalid alignment\n");
        }
        break;
    case 2:
        if (addr & 1) {
            fatal("invalid alignment\n");
        }
        break;
    case 1:
    default:
        break;
        }*/
    if (!same_page(addr, addr + size - 1)) {
        fatal("not on same page\n");
    }
}

uint32_t mem_read(uint32_t addr, size_t size)
{
    uint32_t offset, value;
    struct page_s *page;

    check_alignment(addr, size);

    page = get_page(addr, PAGE_PROT_RO);
    offset = addr - PAGE_START(addr);
    value = *(uint32_t *)&page->data[offset];

    switch (size) {
    case 1:
        value &= 0xff;
        break;
    case 2:
        value &= 0xffff;
        break;
    case 4:
    default:
        break;
    }

    return value;
}

void mem_write(uint32_t addr, size_t size, uint32_t value)
{
    struct page_s *page;
    uint32_t offset;

    check_alignment(addr, size);

    page = get_page(addr, PAGE_PROT_RW);
    offset = addr - PAGE_START(addr);

    switch (size) {
    case 1:
        *(uint8_t *)&page->data[offset] = value & 0xff;
        break;
    case 2:
        *(uint16_t *)&page->data[offset] = value & 0xffff;
        break;
    case 4:
    default:
        *(uint32_t *)&page->data[offset] = value;
        break;
    }
}

static void debug_cpu(uint32_t pc, uint32_t instruction)
{
    char buf[19];

    /*memcpy(buf, "sp: ", 4);
    u32hex(app.cpu.regs[2], &buf[4]);
    buf[12] = '\n';
    buf[13] = '\x00';

    err(buf);*/

    memcpy(buf, "a5: ", 4);
    u32hex(app.cpu.regs[15], &buf[4]);
    buf[12] = '\n';
    buf[13] = '\x00';
    err(buf);

    u32hex(pc, &buf[0]);
    buf[8] = ' ';
    u32hex(instruction, &buf[9]);
    buf[17] = '\n';
    buf[18] = '\x00';

    err(buf);
}


void stream_run_app(void)
{
    uint32_t instruction;

    while (1) {
        instruction = mem_read(app.cpu.pc, sizeof(instruction));
        debug_cpu(app.cpu.pc, instruction);
        rv_cpu_execute(&app.cpu, instruction);
    }
}
