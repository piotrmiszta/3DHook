#ifndef HTTP_PARSER_H_
#define HTTP_PARSER_H_

#include "err_codes.h"
#include "str.h"

enum HttpVersionE
{
    HTTP_VERSION_0_9 = 0,
    HTTP_VERSION_1_0,
    HTTP_VERSION_1_1,
    HTTP_VERSION_2,
    HTTP_VERSION_3,
    HTTP_VERSION_UNKNOWN,
};

enum HttpMethodE
{
    HTTP_METHOD_GET = 0,
    HTTP_METHOD_POST,
    HTTP_METHOD_UNKNOWN,
};

typedef struct HttpHeaderElement
{
    str_view_t key;
    str_view_t value;
} HttpHeaderElement;

typedef struct HttpMessage
{
    str_t message_buffer;
    str_view_t url;
    enum HttpMethodE method;
    enum HttpVersionE version;
    u64 elements_count;
    HttpHeaderElement *elements;
} HttpMessage;

err_t http_message_parse(HttpMessage message[static 1], str_t buff);
err_t http_message_free(HttpMessage message[static 1]);
#endif
