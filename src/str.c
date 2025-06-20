#include <stdlib.h>
#include <string.h>

#include "str.h"


static void *(*internal_alloc)(size_t) = malloc;
static void *(*internal_realloc)(void*, size_t) = realloc;
static void (*internal_free)(void*) = free;

static bool realloc_str(str_t string[static 1], size_t new_size);

void string_register_allocator(void *(*str_alloc)(size_t size),
                               void *(*str_realloc)(void *ptr, size_t size),
                               void (*str_free)(void *ptr)) {
    internal_alloc = str_alloc;
    internal_free = str_free;
    internal_realloc = str_realloc;
}

void string_reset_allocator(void) {
    internal_alloc = malloc;
    internal_realloc = realloc;
    internal_free = free;
}

bool string_is_default_allocator(void) {
    return ((internal_alloc == malloc) &&
            (internal_free == free) &&
            (internal_realloc == realloc));
}

str_t string_create_with_size(size_t size) {
    char* data = internal_alloc(size * sizeof(char));
    if(data == nullptr) {
        size = 0;
    }
    return (str_t) { data, 0, size };
}

str_t string_create_from_cstr(const char cstr[static 1]) {
    size_t size = strlen(cstr);
    char* data = internal_alloc(size * sizeof(char));
    if(data == nullptr) {
        return (str_t) { nullptr, 0, 0, false };
    }
    data = memcpy(data, cstr, size * sizeof(char));
    return (str_t) { data, size, size, true};}

str_t string_create_from_buff(size_t size, const char buffer[static size]) {
    char* data = internal_alloc(size * sizeof(char));
    if(data == nullptr) {
        return (str_t) { nullptr, 0, 0, false };
    }
    data = memcpy(data, buffer, size * sizeof(char));
    return (str_t) { data, size, size, true};
}

str_t string_copy(const str_t string[static 1]) {
    size_t size = string->size;
    char* buffer = string->data;
    char* data = internal_alloc(size * sizeof(char));

    if(data == nullptr) {
        return (str_t) { nullptr, 0, 0, false };
    }
    data = memcpy(data, buffer, size * sizeof(char));
    return (str_t){data, size, size, true};
}

void string_free(str_t string[static 1]) {
    internal_free(string->data);
}

bool string_join(str_t dest[static 1], const str_t src[static 1]) {
    size_t result_size = dest->size + src->size;
    if(dest->capacity < result_size) {
        /* need a realloc */
        if(!realloc_str(dest, result_size)) {
            return false;
        }
    }
    memcpy(&dest->data[dest->size], src->data, src->size);
    return true;
}

void string_to_upper(str_t string[static 1]) {
    char* data = string->data;
    for(size_t i = 0; i < string->size; i++) {
        if(data[i] > 97 && data[i] < 123) {
            data[i] -= 32;
        }
    }
}

void string_to_lower(str_t string[static 1]) {
    char* data = string->data;
    for(size_t i = 0; i < string->size; i++) {
        if(data[i] > 64 && data[i] < 91) {
            data[i] += 32;
        }
    }
}

bool string_equal(const str_t a[static 1], const str_t b[static 1]) {
    if(a->size != b->size) {
        return false;
    }

    if(memcmp(a->data, b->data, a->size) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool string_equal_cstr(const str_t a[static 1], const char *cstr, size_t len) {
    if(a->size != len) {
        return false;
    }

    if(memcmp(a->data, cstr, a->size) == 0) {
        return true;
    }
    else {
        return false;
    }
}

static bool realloc_str(str_t string[static 1], size_t new_size) {
    if(new_size <= string->capacity) {
        return true;
    }
    char* temp = internal_realloc(string->data, new_size);
    if(temp == nullptr) {
        return false;
    }

    string->data = temp;
    string->capacity = new_size;
    return true;
}
