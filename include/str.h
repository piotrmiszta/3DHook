#ifndef STR_H_
#define STR_H_

#include <stddef.h>
#include <stdio.h>
#include <types.h>

#define STRING_NOT_FOUND -101

typedef struct str_t
{
    char *data;
    u64 size;
    u64 capacity;
    bool valid;
} str_t;

typedef struct str_view_t
{
    const char *data;
    u64 size;
    bool valid;
} str_view_t;

typedef struct str_tokenizer_t
{
    const str_view_t *string;
    const str_view_t *delims;
    u64 current_index;
    bool valid;
} str_tokenizer_t;

void string_register_allocator(void *(*str_alloc)(u64 size),
                               void *(*str_realloc)(void *ptr, u64 size),
                               void (*str_free)(void *ptr));
void string_reset_allocator(void);
bool string_is_default_allocator(void);

str_t string_create_with_size(u64 size);
str_t string_create_from_buff(u64 size, const char buffer[static size]);
str_t string_create_from_cstr(const char cst[static 1]);
str_t string_copy(const str_t string[static 1]);
void string_free(str_t string[static 1]);

bool string_join(str_t dest[static 1], const str_t src[static 1]);
void string_to_upper(str_t string[static 1]);
void string_to_lower(str_t string[static 1]);

bool string_equal(const str_t a[static 1], const str_t b[static 1]);
bool string_equal_cstr(const str_t a[static 1], const char *cstr, u64 len);

s64 string_find(const str_t string[static 1], char ch);
s64 string_substr(const str_t string[static 1], const str_t substr[static 1]);

/**
 * @brief replace all 'ch' characters in string by 'val'
 * return number of replaced characters
 */
u64 string_replace(str_t string[static 1], char ch, char val);

str_view_t string_view_create_from_string(const str_t string[static 1]);
bool string_view_equal(const str_view_t a[static 1],
                       const str_view_t b[static 1]);
s64 string_view_find(const str_t string[static 1], char ch);
s64 string_view_substr(const str_t string[static 1],
                       const str_t substr[static 1]);

str_tokenizer_t string_tokenizer_init(const str_view_t string[static 1],
                                      const str_view_t delims[static 1]);
str_view_t string_tokenizer_next(str_tokenizer_t tokenizer[static 1]);
void string_tokenizer_reset(str_tokenizer_t tokenizer[static 1]);

void string_fprintf(FILE *stream, const str_t string[static 1]);
void string_view_fprintf(FILE *stream, const str_view_t string[static 1]);

#define string_view_create_from_cstr(cstr, size)                               \
    {                                                                          \
        (cstr), size, true                                                     \
    }

#define STRING_VIEW_CSTR(str)                                                  \
    (str_view_t) string_view_create_from_cstr((str), sizeof((str)) - 1)

#define CONST_STRING_VIEW_CSTR(str)                                            \
    string_view_create_from_cstr((str), sizeof((str)) - 1)

#endif
