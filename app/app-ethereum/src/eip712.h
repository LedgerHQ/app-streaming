#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#endif

#define DIGEST_SIZE    32
#define MAX_BYTES_SIZE 1024

typedef enum {
    TYPE_BYTES_N,
    TYPE_UINT_N,
    TYPE_INT_N,
    TYPE_BOOL,
    TYPE_ADDRESS,
    TYPE_BYTES,
    TYPE_STRING,
    TYPE_ARRAY,
    TYPE_STRUCT,
} member_type_e;

struct member_data_s;

typedef struct hash_struct_s {
    uint8_t type_hash[DIGEST_SIZE];
    size_t count;
    struct member_data_s *members;
} hash_struct_t;

typedef struct eip712_array_s {
    struct member_data_s *values;
    size_t count;
} eip712_array_t;

typedef struct eip712_address_s {
    char value[40];
} eip712_address_t;

typedef struct eip712_string_s {
    const char *value;
    size_t length;
} eip712_string_t;

typedef struct eip712_bytes_s {
    const char *hex_buffer;
    size_t hex_size;
} eip712_bytes_t;

typedef struct eip712_bytes_n_s {
    char hex[64];
    const size_t count; // [1;32]
} eip712_bytes_n_t;

typedef struct eip712_uint_n_s {
    uint8_t bin[32];
    const size_t count; // [1;32]
} eip712_uint_n_t;

typedef struct eip712_int_n_s {
    uint8_t bin[32];
    const size_t count; // [1;32]
    bool positive;
} eip712_int_n_t;

typedef struct member_data_s {
    member_type_e type;
    char *display;
    union {
        bool boolean;
        eip712_bytes_t bytes;
        eip712_bytes_n_t bytes_n;
        eip712_string_t string;
        hash_struct_t *hstruct;
        eip712_array_t *array;
        eip712_address_t address;
        eip712_int_n_t int_n;
        eip712_uint_n_t uint_n;
    };
} member_data_t;

typedef enum {
    FIELD_KEY,
    FIELD_VALUE,
    FIELD_OTHER,
} json_field_e;

typedef struct json_field_s {
    json_field_e type;
    union {
        char *key;
        member_data_t *value;
        bool other;
    };
} json_field_t;

void copy_string(char *dst, size_t size, eip712_string_t *string);
void copy_address(char *dst, size_t size, eip712_address_t *address);
void copy_amount(uint64_t chain_id, char *dst, size_t size, eip712_uint_n_t *uint);

struct jsmntok;
typedef struct jsmntok jsmntok_t;

const hash_struct_t *eip712_example_mail(const char *json_string, jsmntok_t *t, int token_count);
const char *eip712_hash_struct(const char *json_string,
                               size_t size,
                               const uint8_t *domain,
                               uint8_t *digest);
bool extract_fields(json_field_t *fields,
                    const char *json_string,
                    const jsmntok_t *t,
                    const int token_count);
