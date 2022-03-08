#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <jsmn.h>

#include "crypto.h"
#include "eip712.h"
#include "sdk.h"

#define TOKEN_MAX 128
#define EIP712_DOMAIN_EXAMPLE_MAIL                                                                 \
    "\xf2\xce\xe3\x75\xfa\x42\xb4\x21\x43\x80\x40\x25\xfc\x44\x9d\xea\xfd\x50\xcc\x03\x1c\xa2\x57" \
    "\xe0\xb1\x94\xa6\x50\xa9\x12\x09\x0f"

static void hash_struct(const hash_struct_t *hstruct, uint8_t *digest);

static void encode_address(const eip712_address_t *address, uint8_t *result)
{
    uint8_t bin[20];

    _Static_assert(sizeof(address->value) == 40, "invalid address size");

    memset(result, 0, 12);
    for (size_t i = 0; i < 20; i++) {
        unsigned int c;
        sscanf(&address->value[i * 2], "%02x", &c);
        result[12 + i] = c;
    }
}

static void encode_data(const member_data_t *data, uint8_t *result)
{
    switch (data->type) {
    case TYPE_BOOL:
        if (data->boolean) {
            memcpy(result,
                   "\xb1\x0e\x2d\x52\x76\x12\x07\x3b\x26\xee\xcd\xfd\x71\x7e\x6a\x32\x0c\xf4\x4b"
                   "\x4a\xfa\xc2\xb0\x73\x2d\x9f\xcb\xe2\xb7\xfa\x0c\xf6",
                   32);
        } else {
            memcpy(result,
                   "\x29\x0d\xec\xd9\x54\x8b\x62\xa8\xd6\x03\x45\xa9\x88\x38\x6f\xc8\x4b\xa6\xbc"
                   "\x95\x48\x40\x08\xf6\x36\x2f\x93\x16\x0e\xf3\xe5\x63",
                   32);
        }
        break;
    case TYPE_STRING:
        sha3_256((const uint8_t *)data->string.value, data->string.length, result);
        break;
    case TYPE_BYTES:
        sha3_256(data->bytes.value, data->bytes.size, result);
        break;
    case TYPE_STRUCT:
        hash_struct(data->hstruct, result);
        break;
    case TYPE_ADDRESS:
        encode_address(&data->address, result);
        break;
    default:
        fatal("encode_data: unsupported type (WIP)");
        break;
    }
}

static void hash_struct(const hash_struct_t *hstruct, uint8_t *digest)
{
    ctx_sha3_t ctx;

    sha3_256_init(&ctx);
    sha3_256_update(&ctx, hstruct->type_hash, sizeof(hstruct->type_hash));

    for (size_t i = 0; i < hstruct->count; i++) {
        uint8_t encoded_value[32];
        encode_data(&hstruct->members[i], encoded_value);
        sha3_256_update(&ctx, encoded_value, sizeof(encoded_value));
    }

    sha3_256_final(&ctx, digest);
}

static bool jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
    if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
        return true;
    }
    return false;
}

static void set_string(eip712_string_t *string, const char *value, const size_t size)
{
    string->value = value;
    string->length = size;
}

static bool set_address(eip712_address_t *address, const char *value, const size_t size)
{
    if (size == 40) {
        memcpy(address->value, value, sizeof(address->value));
        return true;
    } else if (size == 42 && strncmp(value, "0x", 2) == 0) {
        memcpy(address->value, value + 2, sizeof(address->value));
        return true;
    } else {
        return false;
    }
}

bool set_value(member_data_t *member, const char *value, const size_t size)
{
    bool success = true;

    switch (member->type) {
    case TYPE_STRING:
        set_string(&member->string, value, size);
        break;
    case TYPE_ADDRESS:
        success = set_address(&member->address, value, size);
        break;
    case TYPE_BYTES:
    default:
        fatal("set_value: unsupported type (WIP)");
        break;
    }

    return success;
}

void copy_string(char *dst, size_t size, eip712_string_t *string)
{
    strncpy(dst, string->value, size);
    if (string->length < size) {
        dst[string->length] = '\x00';
    } else {
        dst[size - 1] = '\x00';
    }
}

void copy_address(char *dst, size_t size, eip712_address_t *address)
{
    size_t len = sizeof(address->value);
    if (len > size - 1) {
        len = size - 1;
    }
    strncpy(dst, address->value, len);
    dst[len - 1] = '\x00';
}

static bool validate_field(json_field_t *field, const char *value, const size_t size)
{
    bool success;

    switch (field->type) {
    case FIELD_VALUE:
        success = set_value(field->value, value, size);
        break;
    case FIELD_KEY:
        success = (strncmp(value, field->key, size) == 0) && (field->key[size] == '\x00');
        break;
    case FIELD_OTHER:
    default:
        success = true;
        break;
    }

    return success;
}

bool extract_fields(json_field_t *fields,
                    const char *json_string,
                    const jsmntok_t *t,
                    const int token_count)
{
    for (int i = 0; i < token_count; i++) {
        if (t[i].type == JSMN_STRING) {
            int start = t[i].start;
            int end = t[i].end;
            size_t size = end - start;
            if (!validate_field(&fields[i], json_string + start, size)) {
                return false;
            }
        }
    }

    return true;
}

/*
 * Computes the digest of a structure given as a JSON string.
 *
 * @return NULL on success, an error string otherwise.
 */
const char *eip712_hash_struct(const char *json_string,
                               size_t size,
                               const uint8_t *domain,
                               uint8_t *digest)
{
    jsmn_parser p;
    jsmn_init(&p);

    jsmntok_t t[TOKEN_MAX];
    int token_count = jsmn_parse(&p, json_string, size, t, TOKEN_MAX);
    if (token_count < 0) {
        return "failed to parse JSON";
    }

    if (token_count < 1 || t[0].type != JSMN_OBJECT) {
        return "JSON object expected";
    }

    const hash_struct_t *hstruct;
    if (memcmp(domain, EIP712_DOMAIN_EXAMPLE_MAIL, 32) == 0) {
        hstruct = eip712_example_mail(json_string, &t[1], token_count - 1);
    } else {
        return "unknown EIP712 domain";
    }

    if (hstruct == NULL) {
        return "failed to handle EIP712";
    }

    hash_struct(hstruct, digest);

    return NULL;
}
