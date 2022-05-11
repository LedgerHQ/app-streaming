#include <stdint.h>
#include <string.h>

#include "cx.h"
#include "os_io_seproxyhal.h"
#include "os_random.h"

#include "rv_cpu.h"

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
};

struct cache_s {
    struct page_s *pages;
    size_t npage;
    bool writeable;
};

struct key_s {
    cx_aes_key_t aes;
    uint8_t hmac[32];
};

struct app_s {
    struct rv_cpu cpu;

    struct section_s sections[NUM_SECTIONS];

    struct page_s code_pages[NPAGE_CODE];
    struct page_s stack_pages[NPAGE_STACK];
    struct page_s data_pages[NPAGE_DATA];

    struct cache_s code;
    struct cache_s stack;
    struct cache_s data;

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

static bool stream_request_page(struct page_s *page, const uint32_t addr, const bool read_only)
{
    struct cmd_request_page_s *cmd = (struct cmd_request_page_s *)G_io_apdu_buffer;
    size_t size;

    app_loading_update_ui(true);

    /* 1. retrieve page data */

    cmd->addr = addr;
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
    page->addr = addr;
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

/**
 * @return true on success, false otherwise
 */
static bool stream_commit_page(struct page_s *page, bool insert)
{
    struct cmd_commit_page_s *cmd1 = (struct cmd_commit_page_s *)G_io_apdu_buffer;
    cx_hmac_sha256_t hmac_sha256_ctx;
    size_t size;

    app_loading_update_ui(true);

    _Static_assert(IO_APDU_BUFFER_SIZE >= sizeof(*cmd1), "invalid IO_APDU_BUFFER_SIZE");

    /* 1. encryption */
    if (page->iv == 0xffffffff) {
        err("iv reuse\n");
        return false;
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
        err("invalid APDU\n");
        return false;
    }

    if (apdu->lc != 0) {
        err("invalid apdu size\n");
        return false;
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
        err("invalid APDU\n");
        return false;
    }

    if ((apdu->lc % sizeof(struct proof_s)) != 0) {
        err("invalid proof size\n");
        return false;
    }

    /* 3. update merkle tree */
    size_t count = apdu->lc / sizeof(struct proof_s);

    if (insert) {
        if (!merkle_insert(&entry, (struct proof_s *)&apdu->data, count)) {
            err("merkle insert failed\n");
            return false;
        }
    } else {
        struct entry_s old_entry;
        old_entry.addr = page->addr;
        old_entry.iv = page->iv - 1;
        if (!merkle_update(&old_entry, &entry, (struct proof_s *)&apdu->data, count)) {
            err("merkle update failed\n");
            return false;
        }
    }

    return true;
}

static void init_cache(struct cache_s *cache,
                       struct page_s *pages,
                       const size_t npage,
                       const bool writeable)
{
    cache->pages = pages;
    cache->npage = npage;
    cache->writeable = writeable;
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

bool stream_init_app(const uint8_t *buffer, const size_t signature_size)
{
    /* 1. store the manifest signature */
    uint8_t signature[72];

    if (signature_size > sizeof(signature)) {
        err("invalid signature\n");
        return false;
    }

    memcpy(signature, buffer, signature_size);

    /* 2. receive the manifest */
    struct cmd_request_manifest_s *cmd = (struct cmd_request_manifest_s *)G_io_apdu_buffer;
    cmd->cmd = (CMD_REQUEST_MANIFEST >> 8) | ((CMD_REQUEST_MANIFEST & 0xff) << 8);

    size_t size = io_exchange(CHANNEL_APDU, sizeof(*cmd));

    struct apdu_s *apdu = parse_apdu(size);
    if (apdu == NULL) {
        err("invalid APDU\n");
        return false;
    }

    const size_t alignment_offset = 3;
    if (apdu->lc != alignment_offset + sizeof(struct manifest_s)) {
        err("invalid manifest APDU\n");
        return false;
    }

    struct manifest_s *manifest = (struct manifest_s *)(apdu->data + alignment_offset);

    /* 3. verify manifest signature */
    if (!verify_manifest_device_signature(manifest, signature, signature_size)) {
        err("invalid manifest\n");
        return false;
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
        err("invalid stack end\n");
        return false;
    }

    app.stack_min = sp;
    app.bss_max = PAGE_START(manifest->bss);

    app.cpu.pc = manifest->pc;
    app.cpu.regs[RV_REG_SP] = sp - 4;

    init_cache(&app.code, app.code_pages, NPAGE_CODE, false);
    init_cache(&app.stack, app.stack_pages, NPAGE_STACK, true);
    init_cache(&app.data, app.data_pages, NPAGE_DATA, true);

    app.current_code_page = NULL;

    init_merkle_tree(manifest->merkle_tree_root_hash, manifest->merkle_tree_size,
                     (struct entry_s *)manifest->last_entry_init);

    lfsr_init();
    sys_app_loading_stop();
    set_app_name(manifest->name);

    return true;
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

/**
 * The page argument is just here to avoid declaring this structure on the stack
 * since this struct is quite large.
 *
 * @return true on success, false otherwise
 */
static bool create_empty_pages(uint32_t from, uint32_t to, struct page_s *page)
{
    uint32_t addr;

    for (addr = from; addr < to; addr += PAGE_SIZE) {
        memset(page->data, '\x00', sizeof(page->data));
        page->addr = addr;
        /* During the first commit, the IV will be incremented to 1 and the
         * dynamic keys will be used for decryption and HMAC. */
        page->iv = 0;
        if (!stream_commit_page(page, true)) {
            return false;
        }
    }

    return true;
}

static int binary_search(const struct page_s *pages, const size_t npage, const uint32_t addr)
{
    int l = 0;
    int r = npage - 1;

    while (l <= r) {
        int m = l + (r - l) / 2;
        if (pages[m].addr == addr) {
            return m;
        } else if (pages[m].addr < addr) {
            l = m + 1;
        } else {
            r = m - 1;
        }
    }

    return -1;
}

/**
 * Sort a set of pages using the insertion sort algorithm.
 *
 * @return the page whose address is given as parameter
 */
static struct page_s *sort_pages(struct page_s *pages, const size_t npage, const uint32_t addr)
{
    size_t n = npage;

    if (pages[0].addr == addr) {
        n = 0;
    }

    for (size_t i = 1; i < npage; i++) {
        struct page_s tmp;

        memcpy(&tmp, &pages[i], sizeof(tmp));

        ssize_t j = i - 1;
        while (j >= 0 && pages[j].addr > tmp.addr) {
            memcpy(&pages[j + 1], &pages[j], sizeof(*pages));
            if (pages[j].addr == addr) {
                n = j + 1;
            }
            j--;
        }

        memcpy(&pages[j + 1], &tmp, sizeof(tmp));
        if (tmp.addr == addr) {
            n = j + 1;
        }
    }

    return (n < npage) ? &pages[n] : NULL;
}

static struct page_s *sort_cache(struct cache_s *cache, const uint32_t addr)
{
    return sort_pages(cache->pages, cache->npage, addr);
}

static struct page_s *find_page(struct cache_s *cache, uint32_t addr)
{
    int n = binary_search(cache->pages, cache->npage, addr);

    if (n < 0) {
        return NULL;
    } else {
        return &cache->pages[n];
    }
}

static struct page_s *choose_page(struct cache_s *cache)
{
    size_t n;

    /* since pages are sorted by their addresses, free pages are first the array entries */
    if (cache->pages[0].addr == 0) {
        n = 0;
    } else {
        n = lfsr_get_random() % cache->npage;
    }

    return &cache->pages[n];
}

static struct page_s *create_page(struct cache_s *cache, const uint32_t addr)
{
    struct page_s *page = choose_page(cache);

    /* don't commit page if it never was retrieved (its address is zero) */
    if (cache->writeable && page->addr != 0) {
        if (!stream_commit_page(page, false)) {
            return NULL;
        }
    }

    /*
     * If a heap/stack page is accessed, create unexisting pages if they don't
     * exist (initialized with zeroes).
     */
    bool zero_page = false;
    if (cache == &app.data) {
        if (addr >= app.bss_max) {
            if (!create_empty_pages(app.bss_max, addr + PAGE_SIZE, page)) {
                return NULL;
            }
            app.bss_max = addr + PAGE_SIZE;
            zero_page = true;
        }
    } else if (cache == &app.stack) {
        if (addr < app.stack_min) {
            if (!create_empty_pages(PAGE_START(addr), app.stack_min, page)) {
                return NULL;
            }
            app.stack_min = addr;
            zero_page = true;
        }
    }

    if (!zero_page) {
        if (!stream_request_page(page, addr, !cache->writeable)) {
            return NULL;
        }
    } else {
        page->addr = addr;
        /* the IV was incremented by 1 during commit */
        page->iv = 1;
        memset(page->data, '\x00', sizeof(page->data));
    }

    if (cache == &app.code) {
        // The code pages will be sorted, which might result in a stale
        // current_code_page pointer.
        app.current_code_page = NULL;
    }

    return sort_cache(cache, page->addr);
}

static struct cache_s *get_cache(const uint32_t addr, const enum page_prot_e page_prot)
{
    if (in_section(SECTION_CODE, addr)) {
        if (page_prot != PAGE_PROT_RO) {
            err("write access to code page\n");
            return NULL;
        }
        return &app.code;
    } else if (in_section(SECTION_DATA, addr)) {
        return &app.data;
    } else if (in_section(SECTION_STACK, addr)) {
        return &app.stack;
    } else {
        err("invalid addr (no section found)\n");
        return NULL;
    }
}

/**
 * @return NULL on error
 */
static struct page_s *get_page(uint32_t addr, enum page_prot_e page_prot)
{
    addr = PAGE_START(addr);

    struct cache_s *cache = get_cache(addr, page_prot);
    if (cache == NULL) {
        return NULL;
    }

    struct page_s *page = find_page(cache, addr);
    if (page != NULL) {
        return page;
    }

    // If the page isn't in the cache, evict a random page from the cache and
    // initialize it.
    return create_page(cache, addr);
}

/**
 * @return true if the buffer of size bytes starting at address addr fits in a
 *         page, false otherwise
 */
static bool fit_in_page(uint32_t addr, size_t size)
{
    if (size == 0) {
        return true;
    }

    if (PAGE_START(addr) != PAGE_START(addr + size - 1)) {
        err("not on same page\n");
        return false;
    }

    return true;
}

static struct page_s *get_code_page(const uint32_t addr)
{
    struct page_s *page = find_page(&app.code, addr);
    if (page != NULL) {
        return page;
    }

    page = choose_page(&app.code);
    if (!stream_request_page(page, addr, true)) {
        return NULL;
    }

    return sort_cache(&app.code, page->addr);
}

static bool get_instruction(const uint32_t addr, uint32_t *instruction)
{
    struct page_s *page;
    uint32_t page_addr;

    if (!fit_in_page(addr, sizeof(uint32_t))) {
        return false;
    }
    page_addr = PAGE_START(addr);

    if (app.current_code_page != NULL && app.current_code_page->addr == page_addr) {
        page = app.current_code_page;
    } else {
        page = get_code_page(page_addr);
        if (page == NULL) {
            // this shouldn't happen
            return false;
        }

        app.current_code_page = page;
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

    if (!fit_in_page(addr, size)) {
        return false;
    }

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

/**
 * @return true on success, false otherwise
 */
bool mem_write(const uint32_t addr, const size_t size, const uint32_t value)
{
    struct page_s *page;
    uint32_t offset;

    if (!fit_in_page(addr, size)) {
        return false;
    }

    page = get_page(addr, PAGE_PROT_RW);
    if (page == NULL) {
        err("get_page failed\n");
        return false;
    }
    offset = addr - PAGE_START(addr);

    if (offset > PAGE_SIZE - size) {
        err("invalid mem_write\n");
        return false;
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

    return true;
}

/**
 * @return NULL on error
 */
uint8_t *get_buffer(const uintptr_t addr, const size_t size, const bool writeable)
{
    if (size == 0 || size > PAGE_SIZE) {
        err("invalid size\n");
        return NULL;
    }

    if (!fit_in_page(addr, size)) {
        err("not on same page\n");
        return NULL;
    }

    struct page_s *page = get_page(addr, writeable ? PAGE_PROT_RW : PAGE_PROT_RO);
    if (page == NULL) {
        err("get_page failed\n");
        return NULL;
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
        stop = !rv_cpu_execute(&app.cpu, instruction);
        app_loading_inc_counter();
    } while (!stop);
}
