#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "cx.h"
#include "os_io_seproxyhal.h"
#include "os_random.h"

#include "apdu.h"
#include "error.h"
#include "keys.h"
#include "merkle.h"
#include "stream.h"

struct cmd_request_next_page_s {
    uint16_t cmd;
} __attribute__((packed));

struct cmd_request_enc_hmac_s {
    uint8_t enc_hmac[32];
    uint16_t cmd;
} __attribute__((packed));

struct cmd_request_signature_s {
    uint8_t aes_key[32];
    uint8_t signature[72];
    uint8_t signature_size;
} __attribute__((packed));

_Static_assert(IO_APDU_BUFFER_SIZE >= sizeof(struct cmd_request_signature_s),
               "invalid struct cmd_request_signature_s");

struct cmd_response_app_s {
    struct manifest_s manifest;
    uint8_t signature[72];
    size_t signature_size;
} __attribute__((packed));

_Static_assert(IO_APDU_BUFFER_SIZE >= sizeof(struct cmd_response_app_s),
               "invalid struct cmd_response_app_s");

static void generate_hmac(const uint32_t addr,
                          const uint8_t *page,
                          const uint8_t *hmac_key,
                          uint8_t *mac)
{
    cx_hmac_sha256_t hmac_sha256_ctx;
    struct entry_s entry;

    cx_hmac_sha256_init_no_throw(&hmac_sha256_ctx, hmac_key, 32);

    entry.addr = addr;
    entry.iv = 0;

    cx_hmac_no_throw((cx_hmac_t *)&hmac_sha256_ctx, 0, page, PAGE_SIZE, NULL, 0);
    cx_hmac_no_throw((cx_hmac_t *)&hmac_sha256_ctx, CX_LAST, entry.data, sizeof(entry.data), mac,
                     32);
}

static void encrypt_hmac(const cx_aes_key_t *aes_ctx, uint8_t *iv, uint8_t *mac)
{
    int flag = CX_CHAIN_CBC | CX_ENCRYPT;
    size_t size = 32;
    cx_aes_iv_no_throw(aes_ctx, flag, iv, CX_AES_BLOCK_SIZE, mac, 32, mac, &size);
    memcpy(iv, mac + CX_AES_BLOCK_SIZE, CX_AES_BLOCK_SIZE);
}

static bool compute_section_hmacs(cx_sha256_t *ctx,
                                  cx_aes_key_t *aes_ctx,
                                  uint8_t *iv,
                                  const uint8_t *hmac_key,
                                  const uint32_t start_addr,
                                  const uint32_t end_addr)
{
    uint32_t addr = start_addr;

    while (addr < end_addr) {
        uint8_t page[PAGE_SIZE];
        uint8_t mac[32];

        /* receive page */
        struct cmd_request_next_page_s *cmd = (struct cmd_request_next_page_s *)G_io_apdu_buffer;
        cmd->cmd = (CMD_REQUEST_APP_PAGE >> 8) | ((CMD_REQUEST_APP_PAGE & 0xff) << 8);
        size_t size = io_exchange(CHANNEL_APDU, sizeof(*cmd));

        struct apdu_s *response = (struct apdu_s *)G_io_apdu_buffer;
        parse_apdu(response, size);

        /* the first byte of the page is in p2 */
        page[0] = response->p2;
        memcpy(&page[1], response->data, PAGE_SIZE - 1);

        /* update app_hash */
        cx_hash_no_throw((cx_hash_t *)ctx, 0, page, PAGE_SIZE, NULL, 0);

        generate_hmac(addr, page, hmac_key, mac);
        encrypt_hmac(aes_ctx, iv, mac);

        /* send encrypted hmac */
        struct cmd_request_enc_hmac_s *cmd_hmac = (struct cmd_request_enc_hmac_s *)G_io_apdu_buffer;
        memcpy(cmd_hmac->enc_hmac, mac, sizeof(mac));
        cmd_hmac->cmd = (CMD_REQUEST_APP_HMAC >> 8) | ((CMD_REQUEST_APP_HMAC & 0xff) << 8);

        size = io_exchange(CHANNEL_APDU, sizeof(*cmd_hmac));
        response = (struct apdu_s *)G_io_apdu_buffer;
        parse_apdu(response, size);

        addr += PAGE_SIZE;
    }

    return true;
}

bool handle_sign_app(const struct cmd_response_app_s *response, size_t *tx)
{
    /* save variables which will be overwritten with G_io_apdu_buffer */
    struct manifest_s manifest;

    memcpy(&manifest, &response->manifest, sizeof(manifest));

    /* verify manifest signature */
    if (!verify_manifest_hsm_signature(&manifest, response->signature, response->signature_size)) {
        return false;
    }

    struct hmac_key_s hmac_key;
    derive_hmac_key(manifest.app_hash, &hmac_key);

    /* init the SHA256 ctx */
    const uint32_t code_start = manifest.sections[SECTION_CODE].start;
    const uint32_t code_end = manifest.sections[SECTION_CODE].end;
    const uint32_t data_start = manifest.sections[SECTION_DATA].start;
    const uint32_t data_end = manifest.bss;
    const uint32_t code_size = code_end - code_start;
    const uint32_t data_size = data_end - data_start;

    cx_sha256_t ctx;
    cx_sha256_init_no_throw(&ctx);
    cx_hash_no_throw((cx_hash_t *)&ctx, 0, (uint8_t *)&code_size, sizeof(code_size), NULL, 0);
    cx_hash_no_throw((cx_hash_t *)&ctx, 0, (uint8_t *)&data_size, sizeof(data_size), NULL, 0);

    /* Generate random AES key to encrypt HMACs. This key will be transmitted to
       the host if and only if the signature over the app manifest and the
       pages is valid. */
    uint8_t aes_key[32];
    cx_aes_key_t aes_ctx;
    uint8_t iv[CX_AES_BLOCK_SIZE] = { 0 };
    cx_get_random_bytes(aes_key, sizeof(aes_key));
    cx_aes_init_key_no_throw(aes_key, sizeof(aes_key), &aes_ctx);

    if (!compute_section_hmacs(&ctx, &aes_ctx, iv, hmac_key.bytes, code_start, code_end)) {
        return false;
    }

    if (!compute_section_hmacs(&ctx, &aes_ctx, iv, hmac_key.bytes, data_start, data_end)) {
        return false;
    }

    explicit_bzero(&hmac_key, sizeof(hmac_key));

    /* verify that the app_hash is valid */
    uint8_t digest[CX_SHA256_SIZE];
    cx_hash_no_throw((cx_hash_t *)&ctx, CX_LAST, NULL, 0, digest, sizeof(digest));

    if (memcmp(manifest.app_hash, digest, sizeof(digest)) != 0) {
        return false;
    }

    struct cmd_request_signature_s *cmd = (struct cmd_request_signature_s *)G_io_apdu_buffer;
    size_t sig_size = sizeof(cmd->signature);
    if (!sign_manifest(&manifest, cmd->signature, &sig_size)) {
        return false;
    }

    cmd->signature_size = sig_size;
    memcpy(cmd->aes_key, aes_key, sizeof(cmd->aes_key));

    *tx = sizeof(*cmd);

    return true;
}
