#ifndef STR_H_
#define STR_H_

#include <stddef.h>

typedef struct str_t {
    char* data;
    size_t size;
    size_t capacity;
    bool valid;
}str_t;

void string_register_allocator(void* (*str_alloc)(size_t size),
                               void* (*str_realloc)(void* ptr, size_t size),
                               void (*str_free)(void* ptr));
void string_reset_allocator(void);
bool string_is_default_allocator(void);

str_t string_create_with_size(size_t size);
str_t string_create_from_cstr(const char cstr[static 1]);
str_t string_create_from_buff(size_t size, const char buffer[static size]);
str_t string_copy(const str_t string[static 1]);
void string_free(str_t string[static 1]);


bool string_join(str_t dest[static 1], const str_t src[static 1]);
void string_to_upper(str_t string[static 1]);
void string_to_lower(str_t string[static 1]);

bool string_equal(const str_t a[static 1], const str_t b[static 1]);
bool string_equal_cstr(const str_t a[static 1], const char* cstr, size_t len);

/**
 * @brief replace all 'ch' characters in string by 'val'
 * return number of replaced characters
 */
size_t string_replace(str_t string[static 1], char ch, char val);

#endif
