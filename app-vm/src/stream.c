#include <stdint.h>
#include <string.h>

#include "cx.h"
#include "os_io_seproxyhal.h"
#include "os_random.h"

#include "rv.h"

#include "apdu.h"
#include "error.h"
#include "lfsr.h"
#include "loading.h"
#include "merkle.h"
#include "stream.h"
#include "ui.h"

#ifdef TARGET_NANOX
#define NPAGE_CODE  70
#define NPAGE_STACK 10
#define NPAGE_DATA  10
#else
#define NPAGE_CODE  3
#define NPAGE_STACK 3
#define NPAGE_DATA  1
#endif

enum page_prot_e {
    PAGE_PROT_RO,
    PAGE_PROT_RW,
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
    uint32_t iv;
    size_t usage;
};

struct section_s {
    uint32_t start;
    uint32_t end;
} __attribute__((packed));

struct key_s {
    cx_aes_key_t aes;
    uint8_t hmac[32];
};

struct app_s {
    struct rv_cpu cpu;

    struct section_s sections[NUM_SECTIONS];

    struct page_s code[NPAGE_CODE];
    struct page_s stack[NPAGE_STACK];
    struct page_s data[NPAGE_DATA];
    struct page_s *current_code_page;

    uint32_t bss_max;
    uint32_t stack_min;

    struct key_s static_key;
    struct key_s dynamic_key;
};

/* this message comes from the client */
struct cmd_app_init_s {
    char name[32];
    uint32_t pc;
    uint32_t bss;

    struct section_s sections[NUM_SECTIONS];

    uint8_t merkle_tree_root_hash[CX_SHA256_SIZE];
    uint32_t merkle_tree_size;
    uint8_t last_entry_init[8];
} __attribute__((packed));

struct cmd_request_page_s {
    uint32_t addr;
    uint16_t cmd;
} __attribute__((packed));

struct cmd_commit_page_s {
    uint8_t data[PAGE_SIZE];
    uint16_t cmd;
} __attribute__((packed));

struct cmd_commit_hmac_s {
    uint32_t addr;
    uint32_t iv;
    uint8_t mac[32];
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

struct response_hmac_s {
    uint32_t iv;
    uint8_t mac[32];
} __attribute__((packed));

static struct app_s app;

static void parse_apdu(const struct response_s *response, size_t size) {
    _Static_assert(IO_APDU_BUFFER_SIZE >= sizeof(*response), "invalid IO_APDU_BUFFER_SIZE");

    if (size < OFFSET_CDATA || size - OFFSET_CDATA != response->lc) {
        fatal("invalid apdu\n");
    }
}

/* This IV is made of:
 * - addr and iv32 to ensure that 2 identical data pages won't result in the
 *   same encrypted data
 * - sha256 of a secret key to prevent attacks on known cleartexts */
static void compute_iv(uint8_t *iv, uint32_t addr, uint32_t iv32)
{
    /* add address to the IV to prevent 2 pages with identical data and iv32
     * from resulting in the same ciphertext */
    memcpy(&iv[0], &addr, sizeof(addr));
    memcpy(&iv[4], &iv32, sizeof(iv32));
    memset(&iv[8], '\x00', CX_AES_BLOCK_SIZE - 8);
}

static void encrypt_page(const void *data, void *out, uint32_t addr, uint32_t iv32)
{
    int flag = CX_CHAIN_CBC | CX_ENCRYPT;
    size_t size = PAGE_SIZE;
    uint8_t iv[CX_AES_BLOCK_SIZE];

    compute_iv(iv, addr, iv32);

    /* Code pages are never commited since they're read-only. Hence, the AES key
     * is always the dynamic one for encryption. */
    cx_aes_iv_no_throw(&app.dynamic_key.aes, flag, iv, CX_AES_BLOCK_SIZE, data, PAGE_SIZE, out, &size);
}

static void decrypt_page(const void *data, void *out, uint32_t addr, uint32_t iv32)
{
    int flag = CX_CHAIN_CBC | CX_DECRYPT;
    size_t size = PAGE_SIZE;
    uint8_t iv[CX_AES_BLOCK_SIZE];
    cx_aes_key_t *key;

    if (iv32 == 0) {
        key = &app.static_key.aes;
    } else {
        key = &app.dynamic_key.aes;
    }

    compute_iv(iv, addr, iv32);
    cx_aes_iv_no_throw(key, flag, iv, CX_AES_BLOCK_SIZE, data, PAGE_SIZE, out, &size);
}

static void init_hmac_ctx(cx_hmac_sha256_t *hmac_sha256_ctx, uint32_t iv)
{
    /*
     * The IV of pages encrypted by the host (read-only and writeable pages) is
     * always 0. The IV of pages encrypted by the VM is always greater than 0.
     */
    if (iv == 0) {
        cx_hmac_sha256_init_no_throw(hmac_sha256_ctx, app.static_key.hmac, sizeof(app.static_key.hmac));
    } else {
        cx_hmac_sha256_init_no_throw(hmac_sha256_ctx, app.dynamic_key.hmac, sizeof(app.dynamic_key.hmac));
    }
}

void stream_request_page(struct page_s *page, bool read_only)
{
    struct cmd_request_page_s *cmd = (struct cmd_request_page_s *)G_io_apdu_buffer;
    size_t size;

    app_loading_update_ui(true);

    /* 1. retrieve page data */

    cmd->addr = page->addr;
    cmd->cmd = (CMD_REQUEST_PAGE >> 8) | ((CMD_REQUEST_PAGE & 0xff) << 8);
    size = io_exchange(CHANNEL_APDU, sizeof(*cmd));

    struct response_s *response = (struct response_s *)G_io_apdu_buffer;
    parse_apdu(response, size);

    if (response->lc != PAGE_SIZE - 1) {
        fatal("invalid response size\n");
    }

    /* the first byte of the page is in p2 */
    page->data[0] = response->p2;
    memcpy(&page->data[1], response->data, PAGE_SIZE - 1);

    /* 2. retrieve and verify hmac */

    cmd->cmd = (CMD_REQUEST_HMAC >> 8) | ((CMD_REQUEST_HMAC & 0xff) << 8);
    size = io_exchange(CHANNEL_APDU, sizeof(*cmd));

    response = (struct response_s *)G_io_apdu_buffer;
    parse_apdu(response, size);

    if (response->lc != sizeof(struct response_hmac_s)) {
        fatal("invalid response size\n");
    }

    struct response_hmac_s *r = (struct response_hmac_s *)&response->data;

    cx_hmac_sha256_t hmac_sha256_ctx;
    struct entry_s entry;
    uint8_t mac[32];

    entry.addr = page->addr;
    entry.iv = r->iv;

    /* TODO: ideally, the IV should be verified before */
    init_hmac_ctx(&hmac_sha256_ctx, entry.iv);
    cx_hmac_no_throw((cx_hmac_t *)&hmac_sha256_ctx, 0, page->data, sizeof(page->data), NULL, 0);
    cx_hmac_no_throw((cx_hmac_t *)&hmac_sha256_ctx, CX_LAST, entry.data, sizeof(entry.data), mac, sizeof(mac));

    if (memcmp(mac, r->mac, sizeof(mac)) != 0) {
        fatal("invalid hmac\n");
    }

    /* 3. decrypt page */
    /* TODO: ideally, decryption should happen before IV verification */

    page->iv = r->iv;
    decrypt_page(page->data, page->data, page->addr, page->iv);

    /* 4. verify iv thanks to the merkle tree if the page is writeable */
    if (read_only) {
        if (page->iv != 0) {
            fatal("invalid id for read-only page\n");
        }
        return;
    }

    cmd->cmd = (CMD_REQUEST_PROOF >> 8) | ((CMD_REQUEST_PROOF & 0xff) << 8);
    size = io_exchange(CHANNEL_APDU, sizeof(*cmd));

    response = (struct response_s *)G_io_apdu_buffer;
    parse_apdu(response, size);

    if ((response->lc % sizeof(struct proof_s)) != 0) {
        fatal("invalid proof size\n");
    }

    size_t count = response->lc / sizeof(struct proof_s);
    if (!merkle_verify_proof(&entry, (struct proof_s *)&response->data, count)) {
        fatal("invalid iv (merkle proof)\n");
    }
}

void stream_commit_page(struct page_s *page, bool insert)
{
    struct cmd_commit_page_s *cmd1 = (struct cmd_commit_page_s *)G_io_apdu_buffer;
    cx_hmac_sha256_t hmac_sha256_ctx;
    size_t size;

    app_loading_update_ui(true);

    _Static_assert(IO_APDU_BUFFER_SIZE >= sizeof(*cmd1), "invalid IO_APDU_BUFFER_SIZE");

    /* 1. encryption */
    if (page->iv == 0xffffffff) {
        fatal("iv reuse\n");
    }
    page->iv++;
    encrypt_page(page->data, cmd1->data, page->addr, page->iv);

    /* initialize hmac here since cmd1->data may be overwritten later */
    init_hmac_ctx(&hmac_sha256_ctx, page->iv);
    cx_hmac_no_throw((cx_hmac_t *)&hmac_sha256_ctx, 0, cmd1->data, sizeof(cmd1->data), NULL, 0);

    cmd1->cmd = (CMD_COMMIT_PAGE >> 8) | ((CMD_COMMIT_PAGE & 0xff) << 8);
    size = io_exchange(CHANNEL_APDU, sizeof(*cmd1));

    struct response_s *response = (struct response_s *)G_io_apdu_buffer;
    parse_apdu(response, size);

    if (response->lc != 0) {
        fatal("invalid response size\n");
    }

    /* 2. hmac(data || addr || iv) */

    struct cmd_commit_hmac_s *cmd2 = (struct cmd_commit_hmac_s *)G_io_apdu_buffer;
    struct entry_s entry;

    _Static_assert(IO_APDU_BUFFER_SIZE >= sizeof(*cmd2), "invalid IO_APDU_BUFFER_SIZE");

    entry.addr = page->addr;
    entry.iv = page->iv;

    cx_hmac_no_throw((cx_hmac_t *)&hmac_sha256_ctx, CX_LAST, entry.data, sizeof(entry.data), cmd2->mac, sizeof(cmd2->mac));

    cmd2->addr = page->addr;
    cmd2->iv = page->iv;
    cmd2->cmd = (CMD_COMMIT_HMAC >> 8) | ((CMD_COMMIT_HMAC & 0xff) << 8);

    size = io_exchange(CHANNEL_APDU, sizeof(*cmd2));

    response = (struct response_s *)G_io_apdu_buffer;
    parse_apdu(response, size);

    if (response->lc == 0 || (response->lc % sizeof(struct proof_s)) != 0) {
        fatal("invalid proof size\n");
    }

    /* 3. update merkle tree */
    size_t count = response->lc / sizeof(struct proof_s);

    if (insert) {
        if (!merkle_insert(&entry, (struct proof_s *)&response->data, count)) {
            fatal("merkle insert failed\n");
        }
    } else {
        struct entry_s old_entry;
        old_entry.addr = page->addr;
        old_entry.iv = page->iv - 1;
        if (!merkle_update(&old_entry, &entry, (struct proof_s *)&response->data, count)) {
            fatal("merkle update failed\n");
        }
    }
}

static void init_static_keys(void)
{
    memset(app.static_key.hmac, 'k', sizeof(app.static_key.hmac));

    uint8_t encryption_key[32];
    memset(encryption_key, 'K', sizeof(encryption_key));
    cx_aes_init_key_no_throw(encryption_key, sizeof(encryption_key), &app.static_key.aes);

    explicit_bzero(encryption_key, sizeof(encryption_key));
}

static void init_dynamic_keys(void)
{
    cx_get_random_bytes(app.dynamic_key.hmac, sizeof(app.dynamic_key.hmac));

    uint8_t encryption_key[32];
    cx_get_random_bytes(encryption_key, sizeof(encryption_key));
    cx_aes_init_key_no_throw(encryption_key, sizeof(encryption_key), &app.dynamic_key.aes);

    explicit_bzero(encryption_key, sizeof(encryption_key));
}

/* TODO: encrypt received parameters */
/* TODO: use different hmac keys for read-only data and else */
void stream_init_app(uint8_t *buffer)
{
    memset(&app, '\x00', sizeof(app));

    init_static_keys();
    init_dynamic_keys();

    struct cmd_app_init_s *cmd = (struct cmd_app_init_s *)buffer;
    memcpy(app.sections, cmd->sections, sizeof(app.sections));

    uint32_t sp = app.sections[SECTION_STACK].end;
    if (PAGE_START(sp) != sp) {
        fatal("invalid stack end\n");
    }

    app.stack_min = sp;
    app.bss_max = PAGE_START(cmd->bss);

    app.cpu.pc = cmd->pc;
    app.cpu.regs[2] = sp - 4;
    app.current_code_page = NULL;

    init_merkle_tree(cmd->merkle_tree_root_hash, cmd->merkle_tree_size, (struct entry_s *)cmd->last_entry_init);

    lfsr_init();
    app_loading_stop();
    set_app_name(cmd->name);
}

static void u32hex(uint32_t n, char *buf)
{
    char hex[16] = "0123456789abcdef";
    size_t i;

    for (i = 0; i < 4; i++) {
        buf[i*2] = hex[(n >> ((24 - i * 8) + 4)) & 0xf];
        buf[i*2+1] = hex[(n >> (24 - i * 8)) & 0xf];
    }
}

static bool in_section(enum section_e section, uint32_t addr)
{
    return addr >= app.sections[section].start && addr < app.sections[section].end;
}

/* the page argument is just here to avoid declaring this structure on the stack
 * since this struct is quite large */
static void create_empty_pages(uint32_t from, uint32_t to, struct page_s *page)
{
    uint32_t addr;

    for (addr = from; addr < to; addr += PAGE_SIZE) {
        memset(page->data, '\x00', sizeof(page->data));
        page->addr = addr;
        /* During the first commit, the IV will be incremented to 1 and the
         * dynamic keys will be used for decryption and HMAC. */
        page->iv = 0;
        stream_commit_page(page, true);
    }
}

static bool find_page(uint32_t addr, struct page_s *pages, size_t npage, struct page_s **result)
{
    struct page_s *page = &pages[0];
    struct page_s *found = NULL;

    for (size_t i = 0; i < npage; i++) {
        /* return the page if address matches */
        if (addr == pages[i].addr) {
            found = &pages[i];
            *result = found;
            return true;
        } else if (pages[i].usage == 0) {
            page = &pages[i];
            *result = page;
            return false;
        }
    }

    size_t n = lfsr_get_random() % npage;
    *result = &pages[n];
    return false;
}

static struct page_s *get_page(uint32_t addr, enum page_prot_e page_prot)
{
    struct page_s *pages, *page;
    size_t npage = 0;
    bool writeable = false;

    addr = PAGE_START(addr);

    if (in_section(SECTION_CODE, addr)) {
        if (page_prot != PAGE_PROT_RO) {
            fatal("write access to code page\n");
        }
        pages = app.code;
        npage = NPAGE_CODE;
    } else if (in_section(SECTION_DATA, addr)) {
        pages = app.data;
        npage = NPAGE_DATA;
        writeable = true;
    } else if (in_section(SECTION_STACK, addr)) {
        pages = app.stack;
        npage = NPAGE_STACK;
        writeable = true;
    } else {
        char buf[19];

        memcpy(buf, "addr: ", 5);
        u32hex(addr, &buf[5]);
        buf[13] = '\n';
        buf[14] = '\x00';
        err(buf);

        memcpy(buf, "gp: ", 4);
        u32hex(app.cpu.regs[3], &buf[4]);
        buf[12] = '\n';
        buf[13] = '\x00';
        err(buf);

        fatal("invalid addr (no section found)\n");
        pages = NULL;
    }

    if (find_page(addr, pages, npage, &page)) {
        return page;
    }

    /* don't commit page if it never was retrieved (its address is zero) */
    if (writeable && page->addr != 0) {
        stream_commit_page(page, false);
    }

    /*
     * If a heap/stack page is accessed, create unexisting pages if they don't
     * exist (initialized with zeroes).
     */
    bool zero_page = false;
    if (pages == app.data) {
        if (addr >= app.bss_max) {
            create_empty_pages(app.bss_max, addr + PAGE_SIZE, page);
            app.bss_max = addr + PAGE_SIZE;
            zero_page = true;
        }
    } else if (pages == app.stack) {
        if (addr < app.stack_min) {
            create_empty_pages(PAGE_START(addr), app.stack_min, page);
            app.stack_min = addr;
            zero_page = true;
        }
    }

    page->addr = addr;
    page->usage = 1;

    if (!zero_page) {
        stream_request_page(page, !writeable);
    } else {
        /* the IV was incremented by 1 during commit */
        page->iv = 1;
        memset(page->data, '\x00', sizeof(page->data));
    }

    return page;
}

static bool same_page(uint32_t addr1, uint32_t addr2)
{
    return PAGE_START(addr1) == PAGE_START(addr2);
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

static uint32_t get_instruction(uint32_t addr)
{
    struct page_s *page;
    uint32_t page_addr;

    check_alignment(addr, sizeof(uint32_t));
    page_addr = PAGE_START(addr);

    if (app.current_code_page != NULL && app.current_code_page->addr == page_addr) {
        page = app.current_code_page;
    } else {
        if (!find_page(page_addr, app.code, NPAGE_CODE, &page)) {
            page->addr = page_addr;
            page->usage = 1;
            stream_request_page(page, true);
            app.current_code_page = page;
        }
    }

    uint32_t offset = addr - page_addr;
    uint32_t value = *(uint32_t *)&page->data[offset];

    return value;
}

uint32_t mem_read(uint32_t addr, size_t size)
{
    uint32_t offset, value;
    struct page_s *page;

    check_alignment(addr, size);

    page = get_page(addr, PAGE_PROT_RO);
    offset = addr - PAGE_START(addr);

    switch (size) {
    case 1:
        value = *(uint8_t *)&page->data[offset];
        break;
    case 2:
        value = *(uint16_t *)&page->data[offset];
        break;
    case 4:
    default:
        value = *(uint32_t *)&page->data[offset];
        break;
    }

    if (0) {
        char buf[32];
        memcpy(buf, "[*] read:  ", 11);
        u32hex(value, &buf[11]);
        buf[19] = '@';
        u32hex(addr, &buf[20]);
        buf[28] = '\n';
        buf[29] = '\x00';
        err(buf);
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

    if (offset > PAGE_SIZE - size) {
        fatal("invalid mem_write\n");
    }

    if (0) {
        char buf[32];
        memcpy(buf, "[*] write: ", 11);
    }

    switch (size) {
    case 1:
        *(uint8_t *)&page->data[offset] = value & 0xff;
        //u32hex(value & 0xff, &buf[11]);
        break;
    case 2:
        *(uint16_t *)&page->data[offset] = value & 0xffff;
        //u32hex(value & 0xffff, &buf[11]);
        break;
    case 4:
    default:
        *(uint32_t *)&page->data[offset] = value;
        //u32hex(value, &buf[11]);
        break;
    }

    /*if (0) {
        buf[19] = '@';
        u32hex(addr, &buf[20]);
        buf[28] = '\n';
        buf[29] = '\x00';
        err(buf);
        }*/
}

uint8_t *get_buffer(uint32_t addr, size_t size, bool writeable)
{
    if (size == 0 || size > PAGE_SIZE) {
        fatal("invalid size\n");
    }

    if (!same_page(addr, addr + size - 1)) {
        fatal("not on same page\n");
    }

    struct page_s *page = get_page(addr, writeable ? PAGE_PROT_RW : PAGE_PROT_RO);
    uint32_t offset = addr - PAGE_START(addr);

    return &page->data[offset];
}

static void debug_cpu(uint32_t pc, uint32_t instruction)
{
    char buf[19];

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
    bool stop;

    do {
        instruction = get_instruction(app.cpu.pc);
        //instruction = mem_read(app.cpu.pc, sizeof(instruction));
        //debug_cpu(app.cpu.pc, instruction);
        stop = rv_cpu_execute(&app.cpu, instruction);
        app_loading_inc_counter();
    } while (!stop);
}
