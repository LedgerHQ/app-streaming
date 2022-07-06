#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "cx.h"
#include "os_io_seproxyhal.h"
#include "os_random.h"

#include "rv_cpu.h"

#include "apdu.h"
#include "error.h"
#include "keys.h"
#include "loading.h"
#include "memory.h"
#include "merkle.h"
#include "stream.h"
#include "ui.h"

struct app_s {
    struct rv_cpu cpu;
    cx_aes_key_t aes_key;
};

struct cmd_request_page_s {
    uint32_t addr;
    uint16_t cmd;
} __attribute__((packed));

struct cmd_request_manifest_s {
    uint16_t cmd;
} __attribute__((packed));

struct cmd_commit_init_s {
    uint32_t addr;
    uint32_t iv;
    uint16_t cmd;
} __attribute__((packed));

struct cmd_commit_page_s {
    uint8_t data[PAGE_SIZE];
    uint16_t cmd;
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
    cx_aes_iv_no_throw(&app.aes_key, flag, iv, CX_AES_BLOCK_SIZE, data, PAGE_SIZE, out,
            &size);
}

static void decrypt_page(const void *data, void *out, uint32_t addr, uint32_t iv32)
{
    int flag = CX_CHAIN_CBC | CX_DECRYPT;
    size_t size = PAGE_SIZE;
    uint8_t iv[CX_AES_BLOCK_SIZE];

    compute_iv(iv, addr, iv32);
    cx_aes_iv_no_throw(&app.aes_key, flag, iv, CX_AES_BLOCK_SIZE, data, PAGE_SIZE, out,
                       &size);
}

bool stream_request_page(struct page_s *page, const uint32_t addr, const bool read_only)
{
    struct cmd_request_page_s *cmd = (struct cmd_request_page_s *)G_io_apdu_buffer;
    size_t size;

    app_loading_update_ui(true);

    _Static_assert(IO_APDU_BUFFER_SIZE >= sizeof(*cmd), "invalid IO_APDU_BUFFER_SIZE");
    _Static_assert(IO_APDU_BUFFER_SIZE >= PAGE_SIZE + 4, "invalid IO_APDU_BUFFER_SIZE");

    /* 1. retrieve page data */

    cmd->addr = addr;
    cmd->cmd = (CMD_REQUEST_PAGE >> 8) | ((CMD_REQUEST_PAGE & 0xff) << 8);
    size = io_exchange(CHANNEL_APDU, sizeof(*cmd));

    if (size != PAGE_SIZE + 4) {
        return false;
    }

    page->addr = addr;
    page->iv = ((uint32_t *)G_io_apdu_buffer)[0];
    memcpy(page->data, &G_io_apdu_buffer[4], PAGE_SIZE);

    /* 2. integrity check using merkle tree */

    cmd->addr = addr;
    cmd->cmd = (CMD_REQUEST_PROOF >> 8) | ((CMD_REQUEST_PROOF & 0xff) << 8);
    size = io_exchange(CHANNEL_APDU, sizeof(*cmd));

    if ((size % sizeof(struct proof_s)) != 0) {
        err("invalid proof size\n");
        return false;
    }

    /* TODO, critical: Find a way to increase the limit caused by merkle proof on the
     *  number of pages stored on the host.

     *  For now, only 2**7 = 128 pages can be stored on the host because each
     *  proof is 33 bytes long and the IO buffer is 260 bytes long.
     *  */
    size_t count = size / sizeof(struct proof_s);
    if (!merkle_verify_proof((struct entry_s *)page, (struct proof_s *)G_io_apdu_buffer, count)) {
        err("invalid merkle proof\n");
        return false;
    }

    /* 3. decrypt page content if needed*/

    if (!read_only && page->iv != 0) {
        decrypt_page(page->data, page->data, page->addr, page->iv);
    }

    return true;
}

/**
 * @return true on success, false otherwise
 */
bool stream_commit_page(struct page_s *page, bool insert)
{
    struct cmd_commit_init_s *cmd_commit_init = (struct cmd_commit_init_s *)G_io_apdu_buffer;
    struct cmd_commit_page_s *cmd_commit_page = (struct cmd_commit_page_s *)G_io_apdu_buffer;
    struct cmd_request_page_s *cmd_request = (struct cmd_request_page_s *)G_io_apdu_buffer;
    size_t size;
    uint8_t proof_buffer[IO_APDU_BUFFER_SIZE];

    app_loading_update_ui(true);

    _Static_assert(IO_APDU_BUFFER_SIZE >= sizeof(*cmd_commit_init), "invalid IO_APDU_BUFFER_SIZE");
    _Static_assert(IO_APDU_BUFFER_SIZE >= sizeof(*cmd_commit_page), "invalid IO_APDU_BUFFER_SIZE");
    _Static_assert(IO_APDU_BUFFER_SIZE >= sizeof(*cmd_request), "invalid IO_APDU_BUFFER_SIZE");
    _Static_assert(IO_APDU_BUFFER_SIZE >= PAGE_SIZE + 4, "invalid IO_APDU_BUFFER_SIZE");

    if (page->iv == ULONG_MAX) {
        err("iv reuse\n");
        return false;
    }
    page->iv++;

    /* 1. send target address and receive proof (to update merkle root) */
    cmd_commit_init->addr = page->addr;
    cmd_commit_init->iv = page->iv;
    cmd_commit_init->cmd = (CMD_COMMIT_INIT >> 8) | ((CMD_COMMIT_INIT & 0xff) << 8);
    size = io_exchange(CHANNEL_APDU, sizeof(*cmd_commit_init));

    if ((size % sizeof(struct proof_s)) != 0) {
        err("invalid proof size\n");
        return false;
    }
    size_t count = size / sizeof(struct proof_s);
    memcpy(proof_buffer, G_io_apdu_buffer, IO_APDU_BUFFER_SIZE);

    /* 2. encrypt and send the new page (code pages are never committed */
    encrypt_page(page->data, page->data, page->addr, page->iv);
    memcpy(cmd_commit_page->data, page->data, PAGE_SIZE);

    cmd_commit_page->cmd = (CMD_COMMIT_PAGE >> 8) | ((CMD_COMMIT_PAGE & 0xff) << 8);
    size = io_exchange(CHANNEL_APDU, sizeof(*cmd_commit_page));

    /* 3. get previous page (only when updating) and update merkle root */
    if (insert) {
        if (!merkle_insert((struct entry_s *)page, (struct proof_s *)proof_buffer, count)) {
            err("merkle insert failed\n");
            return false;
        }
    } else {
        if (size != PAGE_SIZE + 4) {
            err("invalid page size\n");
            return false;
        }

        struct page_s old_page;
        /* Very important: this ensures that we are replacing the right leaf
         * in the merkle tree */
        old_page.addr = page->addr;
        old_page.iv = ((uint32_t *)G_io_apdu_buffer)[0];
        memcpy(old_page.data, &G_io_apdu_buffer[4], PAGE_SIZE);

        if (!merkle_update(
                    (struct entry_s *)&old_page, (struct entry_s *)page,
                    (struct proof_s *)proof_buffer, count)) {
            err("merkle update failed\n");
            return false;
        }
    }

    return true;
}

static void init_encryption_key(void)
{
    uint8_t encryption_key[32];
    cx_get_random_bytes(encryption_key, sizeof(encryption_key));
    cx_aes_init_key_no_throw(encryption_key, sizeof(encryption_key), &app.aes_key);

    explicit_bzero(encryption_key, sizeof(encryption_key));
}

bool stream_init_app(const uint8_t *buffer, const size_t signature_size)
{
    _Static_assert(IO_APDU_BUFFER_SIZE >= sizeof(struct manifest_s), "invalid IO_APDU_BUFFER_SIZE");

    /* 1. store the manifest signature */
    uint8_t signature[72];

    if (signature_size > sizeof(signature)) {
        err("invalid signature size\n");
        return false;
    }

    memcpy(signature, buffer, signature_size);

    /* 2. receive the manifest */
    struct cmd_request_manifest_s *cmd = (struct cmd_request_manifest_s *)G_io_apdu_buffer;
    cmd->cmd = (CMD_REQUEST_MANIFEST >> 8) | ((CMD_REQUEST_MANIFEST & 0xff) << 8);

    size_t size = io_exchange(CHANNEL_APDU, sizeof(*cmd));

    if (size != sizeof(struct manifest_s)) {
        err("invalid manifest size\n");
        return false;
    }

    struct manifest_s *manifest = (struct manifest_s *)G_io_apdu_buffer;

    /* 3. verify manifest signature */
    if (!verify_manifest_hsm_signature(manifest, signature, signature_size)) {
        err("invalid manifest signature\n");
        return false;
    }

    memset(&app, '\x00', sizeof(app));

    /* 4. init encryption key */
    init_encryption_key();

    /* 5. initialize app characteristics from the manifest */
    init_memory_sections(manifest->sections);

    uint32_t sp = manifest->sections[SECTION_STACK].end;
    if (PAGE_START(sp) != sp) {
        err("invalid stack end\n");
        return false;
    }

    init_memory_addresses(manifest->bss, sp);

    app.cpu.pc = manifest->pc;

    /*
     * Align the stack pointer to 16-bytes. According to
     * https://riscv.org/wp-content/uploads/2015/01/riscv-calling.pdf:
     *
     *   In the standard RISC-V calling convention, the stack grows downward and
     *   the stack pointer is always kept 16-byte aligned.
     */
    app.cpu.regs[RV_REG_SP] = (sp - 4) & 0xfffffff0;

    init_caches();

    init_merkle_tree(manifest->merkle_tree_root_hash, manifest->merkle_tree_size,
                     manifest->last_entry_digest_init);

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

static bool get_instruction(const uint32_t addr, uint32_t *instruction)
{
    if (!fit_in_page(addr, sizeof(uint32_t))) {
        return false;
    }

    uint32_t page_addr = PAGE_START(addr);

    struct page_s *page = get_code_page(page_addr);
    if (page == NULL) {
        // this shouldn't happen
        return false;
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
        err("get_page (mem_read) failed\n");
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
        err("get_page (mem_write) failed\n");
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
        err("get_page (get_buffer) failed\n");
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
