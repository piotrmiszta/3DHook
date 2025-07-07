#include "http_parser.h"
#include "err_codes.h"
#include "str.h"
#include <stdlib.h>

constexpr u64 max_header_elements = 100;

str_view_t versions[] = {
    CONST_STRING_VIEW_CSTR("HTTP/0.9"), CONST_STRING_VIEW_CSTR("HTTP/1.0"),
    CONST_STRING_VIEW_CSTR("HTTP/1.1"), CONST_STRING_VIEW_CSTR("HTTP/2"),
    CONST_STRING_VIEW_CSTR("HTTP/3"),
};

str_view_t methods[] = {
    CONST_STRING_VIEW_CSTR("GET"),
    CONST_STRING_VIEW_CSTR("POST"),
};

static enum HttpVersionE http_get_version_from_string(str_view_t string);
static enum HttpMethodE http_get_method_from_string(str_view_t string);

err_t http_message_parse(HttpMessage message[static 1], str_t buff)
{
    message->message_buffer = buff;
    str_view_t buffer = string_view_create_from_string(&buff);
    str_view_t delims = STRING_VIEW_CSTR("\n:");
    str_tokenizer_t tokenizer = string_tokenizer_init(&buffer, &delims);
    str_view_t first_line = string_tokenizer_next(&tokenizer);

    auto tokenizer_first_line =
        string_tokenizer_init(&first_line, &STRING_VIEW_CSTR(" "));

    str_view_t method = string_tokenizer_next(&tokenizer_first_line);
    str_view_t url = string_tokenizer_next(&tokenizer_first_line);
    str_view_t version = string_tokenizer_next(&tokenizer_first_line);
    message->version = http_get_version_from_string(version);
    message->method = http_get_method_from_string(method);
    message->url = url;

    HttpHeaderElement elements[max_header_elements];
    message->elements_count = 0;
    while (true)
    {
        str_view_t key = string_tokenizer_next(&tokenizer);
        if (key.data == nullptr)
        {
            break;
        }
        str_view_t value = string_tokenizer_next(&tokenizer);
        string_view_remove_whitespaces(&key);
        string_view_remove_whitespaces(&value);
        HttpHeaderElement el = {.key = key, .value = value};
        elements[message->elements_count] = el;
        message->elements_count++;
    }

    message->elements =
        malloc(sizeof(*message->elements) * message->elements_count);
    if (message->elements == nullptr)
    {
        return EMEMORY;
    }

    memcpy(message->elements, elements,
           sizeof(*elements) * message->elements_count);

    return SUCCESS;
}

static enum HttpVersionE http_get_version_from_string(str_view_t string)
{
    for (u64 i = 0; i < sizeof(versions) / sizeof(versions[0]); i++)
    {
        if (string_view_equal(&string, &versions[i]))
        {
            return i;
        }
    }
    return HTTP_VERSION_UNKNOWN;
}

static enum HttpMethodE http_get_method_from_string(str_view_t string)
{
    for (u64 i = 0; i < sizeof(methods) / sizeof(methods[0]); i++)
    {
        if (string_view_equal(&string, &methods[i]))
        {
            return i;
        }
    }
    return HTTP_METHOD_UNKNOWN;
}
