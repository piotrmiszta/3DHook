#include <stdlib.h>
#include <string.h>

#include "str.h"

static void *(*internal_alloc)(u64) = malloc;
static void *(*internal_realloc)(void *, u64) = realloc;
static void (*internal_free)(void *) = free;

static bool realloc_str(str_t string[static 1], u64 new_size);

void string_register_allocator(void *(*str_alloc)(u64 size), void *(*str_realloc)(void *ptr, u64 size),
                               void (*str_free)(void *ptr))
{
    internal_alloc = str_alloc;
    internal_free = str_free;
    internal_realloc = str_realloc;
}

void string_reset_allocator(void)
{
    internal_alloc = malloc;
    internal_realloc = realloc;
    internal_free = free;
}

bool string_is_default_allocator(void)
{
    return ((internal_alloc == malloc) && (internal_free == free) && (internal_realloc == realloc));
}

str_t string_create_with_size(u64 size)
{
    char *data = internal_alloc(size * sizeof(char));
    if (data == nullptr)
    {
        size = 0;
    }
    return (str_t){data, 0, size};
}

str_t string_create_from_cstr(const char cstr[static 1])
{
    u64 size = strlen(cstr);
    char *data = internal_alloc(size * sizeof(char));
    if (data == nullptr)
    {
        return (str_t){nullptr, 0, 0, false};
    }
    data = memcpy(data, cstr, size * sizeof(char));
    return (str_t){data, size, size, true};
}

str_t string_create_from_buff(u64 size, const char buffer[static size])
{
    char *data = internal_alloc(size * sizeof(char));
    if (data == nullptr)
    {
        return (str_t){nullptr, 0, 0, false};
    }
    data = memcpy(data, buffer, size * sizeof(char));
    return (str_t){data, size, size, true};
}

str_t string_copy(const str_t string[static 1])
{
    u64 size = string->size;
    char *buffer = string->data;
    char *data = internal_alloc(size * sizeof(char));

    if (data == nullptr)
    {
        return (str_t){nullptr, 0, 0, false};
    }
    data = memcpy(data, buffer, size * sizeof(char));
    return (str_t){data, size, size, true};
}

void string_free(str_t string[static 1])
{
    internal_free(string->data);
}

bool string_join(str_t dest[static 1], const str_t src[static 1])
{
    u64 result_size = dest->size + src->size;
    if (dest->capacity < result_size)
    {
        /* need a realloc */
        if (!realloc_str(dest, result_size))
        {
            return false;
        }
    }
    memcpy(&dest->data[dest->size], src->data, src->size);
    return true;
}

void string_to_upper(str_t string[static 1])
{
    char *data = string->data;
    for (u64 i = 0; i < string->size; i++)
    {
        if (data[i] > 97 && data[i] < 123)
        {
            data[i] -= 32;
        }
    }
}

void string_to_lower(str_t string[static 1])
{
    char *data = string->data;
    for (u64 i = 0; i < string->size; i++)
    {
        if (data[i] > 64 && data[i] < 91)
        {
            data[i] += 32;
        }
    }
}

bool string_equal(const str_t a[static 1], const str_t b[static 1])
{
    if (a->size != b->size)
    {
        return false;
    }

    if (memcmp(a->data, b->data, a->size) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool string_equal_cstr(const str_t a[static 1], const char *cstr, u64 len)
{
    if (a->size != len)
    {
        return false;
    }

    if (memcmp(a->data, cstr, a->size) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

static bool realloc_str(str_t string[static 1], u64 new_size)
{
    if (new_size <= string->capacity)
    {
        return true;
    }
    char *temp = internal_realloc(string->data, new_size);
    if (temp == nullptr)
    {
        return false;
    }

    string->data = temp;
    string->capacity = new_size;
    return true;
}

s64 string_find(const str_t string[static 1], char ch)
{
    for (u64 i = 0; i < string->size; i++)
    {
        if (string->data[i] == ch)
        {
            return i;
        }
    }
    return STRING_NOT_FOUND;
}

s64 string_substr(const str_t string[static 1], const str_t substr[static 1])
{
    s64 found = STRING_NOT_FOUND;
    for (u64 i = 0; i < string->size; i++)
    {
        found = i;
        for (u64 j = 0; j < substr->size; j++)
        {
            if (string->data[i + j] != substr->data[j])
            {
                found = STRING_NOT_FOUND;
                break;
            }
        }
        if (found >= 0)
        {
            return found;
        }
    }
    return found;
}

str_view_t string_view_create_from_string(const str_t string[static 1])
{
    if (string->data == nullptr)
    {
        return (str_view_t){nullptr, 0, false};
    }
    else
    {
        return (str_view_t){string->data, string->size, true};
    }
}

str_view_t string_view_create_from_cstr(const char data[static 1], u64 size)
{
    return (str_view_t){data, size, true};
}

bool string_view_equal(const str_view_t a[static 1], const str_view_t b[static 1])
{
    if (a->size != b->size)
    {
        return false;
    }

    if (memcmp(a->data, b->data, a->size) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

s64 string_view_find(const str_t string[static 1], char ch)
{
    for (u64 i = 0; i < string->size; i++)
    {
        if (string->data[i] == ch)
        {
            return i;
        }
    }
    return STRING_NOT_FOUND;
}
s64 string_view_substr(const str_t string[static 1], const str_t substr[static 1])
{
    s64 found = STRING_NOT_FOUND;
    for (u64 i = 0; i < string->size; i++)
    {
        found = i;
        for (u64 j = 0; j < substr->size; j++)
        {
            if (string->data[i + j] != substr->data[j])
            {
                found = STRING_NOT_FOUND;
                break;
            }
        }
        if (found >= 0)
        {
            return found;
        }
    }
    return found;
}

str_tokenizer_t string_tokenizer_init(const str_view_t string[static 1], const str_view_t delims[static 1])
{
    return (str_tokenizer_t){string, delims, 0, true};
}

str_view_t string_tokenizer_next(str_tokenizer_t tokenizer[static 1])
{
    u64 start = 0;
    if (tokenizer->valid == false)
    {
        return (str_view_t){nullptr, 0, false};
    }
    if (tokenizer->current_index != 0)
    {
        tokenizer->current_index += 1;
        start = tokenizer->current_index;
    }
    for (u64 i = start; i < tokenizer->string->size; i++)
    {
        for (u64 j = 0; j < tokenizer->delims->size; j++)
        {
            if (tokenizer->string->data[i] == tokenizer->delims->data[j])
            {
                str_view_t res = {&tokenizer->string->data[tokenizer->current_index], i - tokenizer->current_index,
                                  true};
                tokenizer->current_index = i;
                return res;
            }
        }
    }
    if (tokenizer->valid && tokenizer->current_index != tokenizer->string->size)
    {
        str_view_t res = {&tokenizer->string->data[tokenizer->current_index],
                          tokenizer->string->size - tokenizer->current_index, true};
        tokenizer->current_index = tokenizer->string->size;
        tokenizer->valid = false;
        return res;
    }
    return (str_view_t){nullptr, 0, false};
}

void string_tokenizer_reset(str_tokenizer_t tokenizer[static 1])
{
    tokenizer->current_index = 0;
    tokenizer->valid = true;
}
