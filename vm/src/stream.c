#include <stdint.h>
#include <string.h>

#include "cx.h"
#include "os_io_seproxyhal.h"
#include "os_random.h"

#include "rv.h"

#include "apdu.h"
#include "error.h"
#include "keys.h"
#include "lfsr.h"
#include "loading.h"
#include "merkle.h"
#include "stream.h"
#include "ui.h"

#ifdef TARGET_NANOX
#define NPAGE_CODE  65
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

struct page_s {
    uint32_t addr;
    uint8_t data[PAGE_SIZE];
    uint32_t iv;
    size_t usage;
};

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

    uint8_t hmac_static_key[32];
    struct key_s dynamic_key;
};

struct cmd_request_page_s {
    uint32_t addr;
    uint16_t cmd;
} __attribute__((packed));

struct cmd_request_manifest_s {
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

struct response_hmac_s {
    uint32_t iv;
    uint8_t mac[32];
} __attribute__((packed));

static struct app_s app;

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
    cx_aes_iv_no_throw(&app.dynamic_key.aes, flag, iv, CX_AES_BLOCK_SIZE, data, PAGE_SIZE, out,
                       &size);
}

static void decrypt_page(const void *data, void *out, uint32_t addr, uint32_t iv32)
{
    int flag = CX_CHAIN_CBC | CX_DECRYPT;
    size_t size = PAGE_SIZE;
    uint8_t iv[CX_AES_BLOCK_SIZE];

    compute_iv(iv, addr, iv32);
    cx_aes_iv_no_throw(&app.dynamic_key.aes, flag, iv, CX_AES_BLOCK_SIZE, data, PAGE_SIZE, out,
                       &size);
}

static void init_hmac_ctx(cx_hmac_sha256_t *hmac_sha256_ctx, uint32_t iv)
{
    /*
     * The IV of pages encrypted by the host (read-only and writeable pages) is
     * always 0. The IV of pages encrypted by the VM is always greater than 0.
     */
    if (iv == 0) {
        cx_hmac_sha256_init_no_throw(hmac_sha256_ctx, app.hmac_static_key,
                                     sizeof(app.hmac_static_key));
    } else {
        cx_hmac_sha256_init_no_throw(hmac_sha256_ctx, app.dynamic_key.hmac,
                                     sizeof(app.dynamic_key.hmac));
    }
}

static bool stream_request_page(struct page_s *page, bool read_only)
{
    struct cmd_request_page_s *cmd = (struct cmd_request_page_s *)G_io_apdu_buffer;
    size_t size;

    app_loading_update_ui(true);

    /* 1. retrieve page data */

    cmd->addr = page->addr;
    cmd->cmd = (CMD_REQUEST_PAGE >> 8) | ((CMD_REQUEST_PAGE & 0xff) << 8);
    size = io_exchange(CHANNEL_APDU, sizeof(*cmd));

    struct apdu_s *apdu = parse_apdu(size);
    if (apdu == NULL) {
        err("invalid APDU\n");
        return false;
    }

    if (apdu->lc != PAGE_SIZE - 1) {
        err("invalid apdu size\n");
        return false;
    }

    /* the first byte of the page is in p2 */
    page->data[0] = apdu->p2;
    memcpy(&page->data[1], apdu->data, PAGE_SIZE - 1);

    /* 2. retrieve and verify hmac */

    cmd->cmd = (CMD_REQUEST_HMAC >> 8) | ((CMD_REQUEST_HMAC & 0xff) << 8);
    size = io_exchange(CHANNEL_APDU, sizeof(*cmd));

    apdu = parse_apdu(size);
    if (apdu == NULL) {
        err("invalid APDU\n");
        return false;
    }

    if (apdu->lc != sizeof(struct response_hmac_s)) {
        err("invalid apdu size\n");
        return false;
    }

    struct response_hmac_s *r = (struct response_hmac_s *)&apdu->data;

    cx_hmac_sha256_t hmac_sha256_ctx;
    struct entry_s entry;
    uint8_t mac[32];

    entry.addr = page->addr;
    entry.iv = r->iv;

    /* TODO: ideally, the IV should be verified before */
    init_hmac_ctx(&hmac_sha256_ctx, entry.iv);
    cx_hmac_no_throw((cx_hmac_t *)&hmac_sha256_ctx, 0, page->data, sizeof(page->data), NULL, 0);
    cx_hmac_no_throw((cx_hmac_t *)&hmac_sha256_ctx, CX_LAST, entry.data, sizeof(entry.data), mac,
                     sizeof(mac));

    if (memcmp(mac, r->mac, sizeof(mac)) != 0) {
        err("invalid hmac\n");
        return false;
    }

    /* 3. decrypt page */
    /* TODO: ideally, decryption should happen before IV verification */

    page->iv = r->iv;
    if (page->iv != 0) {
        decrypt_page(page->data, page->data, page->addr, page->iv);
    }

    /* 4. verify iv thanks to the merkle tree if the page is writeable */
    if (read_only) {
        if (page->iv != 0) {
            err("invalid id for read-only page\n");
            return false;
        }
        return true;
    }

    cmd->cmd = (CMD_REQUEST_PROOF >> 8) | ((CMD_REQUEST_PROOF & 0xff) << 8);
    size = io_exchange(CHANNEL_APDU, sizeof(*cmd));

    apdu = parse_apdu(size);
    if (apdu == NULL) {
        err("invalid APDU\n");
        return false;
    }

    if ((apdu->lc % sizeof(struct proof_s)) != 0) {
        err("invalid proof size\n");
        return false;
    }

    size_t count = apdu->lc / sizeof(struct proof_s);
    if (!merkle_verify_proof(&entry, (struct proof_s *)&apdu->data, count)) {
        err("invalid iv (merkle proof)\n");
        return false;
    }

    return true;
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

    struct apdu_s *apdu = parse_apdu(size);
    if (apdu == NULL) {
        fatal("invalid APDU\n");
    }

    if (apdu->lc != 0) {
        fatal("invalid apdu size\n");
    }

    /* 2. hmac(data || addr || iv) */

    struct cmd_commit_hmac_s *cmd2 = (struct cmd_commit_hmac_s *)G_io_apdu_buffer;
    struct entry_s entry;

    _Static_assert(IO_APDU_BUFFER_SIZE >= sizeof(*cmd2), "invalid IO_APDU_BUFFER_SIZE");

    entry.addr = page->addr;
    entry.iv = page->iv;

    cx_hmac_no_throw((cx_hmac_t *)&hmac_sha256_ctx, CX_LAST, entry.data, sizeof(entry.data),
                     cmd2->mac, sizeof(cmd2->mac));

    cmd2->addr = page->addr;
    cmd2->iv = page->iv;
    cmd2->cmd = (CMD_COMMIT_HMAC >> 8) | ((CMD_COMMIT_HMAC & 0xff) << 8);

    size = io_exchange(CHANNEL_APDU, sizeof(*cmd2));

    apdu = parse_apdu(size);
    if (apdu == NULL) {
        fatal("invalid APDU\n");
    }

    if ((apdu->lc % sizeof(struct proof_s)) != 0) {
        fatal("invalid proof size\n");
    }

    /* 3. update merkle tree */
    size_t count = apdu->lc / sizeof(struct proof_s);

    if (insert) {
        if (!merkle_insert(&entry, (struct proof_s *)&apdu->data, count)) {
            fatal("merkle insert failed\n");
        }
    } else {
        struct entry_s old_entry;
        old_entry.addr = page->addr;
        old_entry.iv = page->iv - 1;
        if (!merkle_update(&old_entry, &entry, (struct proof_s *)&apdu->data, count)) {
            fatal("merkle update failed\n");
        }
    }
}

static void init_static_key(struct hmac_key_s *key)
{
    memcpy(app.hmac_static_key, key->bytes, sizeof(app.hmac_static_key));
}

static void init_dynamic_keys(void)
{
    cx_get_random_bytes(app.dynamic_key.hmac, sizeof(app.dynamic_key.hmac));

    uint8_t encryption_key[32];
    cx_get_random_bytes(encryption_key, sizeof(encryption_key));
    cx_aes_init_key_no_throw(encryption_key, sizeof(encryption_key), &app.dynamic_key.aes);

    explicit_bzero(encryption_key, sizeof(encryption_key));
}

void stream_init_app(uint8_t *buffer, size_t signature_size)
{
    /* 1. store the manifest signature */
    uint8_t signature[72];

    if (signature_size > sizeof(signature)) {
        fatal("invalid signature\n");
    }

    memcpy(signature, buffer, signature_size);

    /* 2. receive the manifest */
    struct cmd_request_manifest_s *cmd = (struct cmd_request_manifest_s *)G_io_apdu_buffer;
    cmd->cmd = (CMD_REQUEST_MANIFEST >> 8) | ((CMD_REQUEST_MANIFEST & 0xff) << 8);

    size_t size = io_exchange(CHANNEL_APDU, sizeof(*cmd));

    struct apdu_s *apdu = parse_apdu(size);
    if (apdu == NULL) {
        fatal("invalid APDU\n");
    }

    const size_t alignment_offset = 3;
    if (apdu->lc != alignment_offset + sizeof(struct manifest_s)) {
        fatal("invalid manifest APDU\n");
    }

    struct manifest_s *manifest = (struct manifest_s *)(apdu->data + alignment_offset);

    /* 3. verify manifest signature */
    if (!verify_manifest_device_signature(manifest, signature, signature_size)) {
        fatal("invalid manifest\n");
    }

    memset(&app, '\x00', sizeof(app));

    /* 4. derive and init keys depending on the app hash */
    struct hmac_key_s hmac_key;
    derive_hmac_key(manifest->app_hash, &hmac_key);
    init_static_key(&hmac_key);
    explicit_bzero(&hmac_key, sizeof(hmac_key));

    init_dynamic_keys();

    /* 5. initialize app characteristics from the manifest */
    memcpy(app.sections, manifest->sections, sizeof(app.sections));

    uint32_t sp = app.sections[SECTION_STACK].end;
    if (PAGE_START(sp) != sp) {
        fatal("invalid stack end\n");
    }

    app.stack_min = sp;
    app.bss_max = PAGE_START(manifest->bss);

    app.cpu.pc = manifest->pc;
    app.cpu.regs[RV_REG_SP] = sp - 4;
    app.current_code_page = NULL;

    init_merkle_tree(manifest->merkle_tree_root_hash, manifest->merkle_tree_size,
                     (struct entry_s *)manifest->last_entry_init);

    lfsr_init();
    sys_app_loading_stop();
    set_app_name(manifest->name);
}

static void u32hex(uint32_t n, char *buf)
{
    char hex[16] = "0123456789abcdef";
    size_t i;

    for (i = 0; i < 4; i++) {
        buf[i * 2] = hex[(n >> ((24 - i * 8) + 4)) & 0xf];
        buf[i * 2 + 1] = hex[(n >> (24 - i * 8)) & 0xf];
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

/**
 * @return NULL on error
 */
static struct page_s *get_page(uint32_t addr, enum page_prot_e page_prot)
{
    struct page_s *pages, *page;
    size_t npage = 0;
    bool writeable = false;

    addr = PAGE_START(addr);

    if (in_section(SECTION_CODE, addr)) {
        if (page_prot != PAGE_PROT_RO) {
            err("write access to code page\n");
            return NULL;
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
        err("invalid addr (no section found)\n");
        return NULL;
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
        if (!stream_request_page(page, !writeable)) {
            return NULL;
        }
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
    if (!same_page(addr, addr + size - 1)) {
        fatal("not on same page\n");
    }
}

static bool get_instruction(const uint32_t addr, uint32_t *instruction)
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
            if (!stream_request_page(page, true)) {
                return false;
            }
            app.current_code_page = page;
        }
    }

    uint32_t offset = addr - page_addr;
    uint32_t value = *(uint32_t *)&page->data[offset];

    *instruction = value;

    return true;
}

/**
 * @return true on success, false otherwise
 */
bool mem_read(const uint32_t addr, const size_t size, uint32_t *value)
{
    struct page_s *page;
    uint32_t offset;

    check_alignment(addr, size);

    page = get_page(addr, PAGE_PROT_RO);
    if (page == NULL) {
        err("get_page failed\n");
        return false;
    }
    offset = addr - PAGE_START(addr);

    switch (size) {
    case 1:
        *value = *(uint8_t *)&page->data[offset];
        break;
    case 2:
        *value = *(uint16_t *)&page->data[offset];
        break;
    case 4:
    default:
        *value = *(uint32_t *)&page->data[offset];
        break;
    }

    return true;
}

void mem_write(uint32_t addr, size_t size, uint32_t value)
{
    struct page_s *page;
    uint32_t offset;

    check_alignment(addr, size);

    page = get_page(addr, PAGE_PROT_RW);
    if (page == NULL) {
        fatal("get_page failed\n");
    }
    offset = addr - PAGE_START(addr);

    if (offset > PAGE_SIZE - size) {
        fatal("invalid mem_write\n");
    }

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

uint8_t *get_buffer(uint32_t addr, size_t size, bool writeable)
{
    if (size == 0 || size > PAGE_SIZE) {
        fatal("invalid size\n");
    }

    if (!same_page(addr, addr + size - 1)) {
        fatal("not on same page\n");
    }

    struct page_s *page = get_page(addr, writeable ? PAGE_PROT_RW : PAGE_PROT_RO);
    if (page == NULL) {
        fatal("get_page failed\n");
    }

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
        if (!get_instruction(app.cpu.pc, &instruction)) {
            fatal("get_instruction failed\n");
        }
        if (0) {
            debug_cpu(app.cpu.pc, instruction);
        }
        stop = rv_cpu_execute(&app.cpu, instruction);
        app_loading_inc_counter();
    } while (!stop);
}
