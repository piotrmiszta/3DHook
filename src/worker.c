#include "worker.h"
#include "err_codes.h"
#include "http_parser.h"
#include "str.h"
#include "utils.h"
#include <stdio.h>

static str_view_t path_to_root = CONST_STRING_VIEW_CSTR("/var/www/html");

static err_t worker_process_get(HttpMessage msg[static 1], s32 client_fd,
                                str_t result[static 1]);

err_t worker_process(HttpMessage msg[static 1], s32 client_fd,
                     str_t result[static 1])
{
    switch (msg->method)
    {
    case HTTP_METHOD_GET:
        return worker_process_get(msg, client_fd, result);
    case HTTP_METHOD_POST:
    case HTTP_METHOD_UNKNOWN:
        return EHTTP_METHOD;
    }
    return SUCCESS;
}

static err_t worker_process_get(HttpMessage msg[static 1], s32 client_fd,
                                str_t result[static 1])
{
    ASSERT(msg->method == HTTP_METHOD_GET,
           "msg method is not equal to msg_get in worker_process_get()");
    char buffer[256] = {0};
    str_view_t path = string_view_join(&path_to_root, &msg->url, buffer, 256);
    if (!path.data)
    {
        return EMEMORY;
    }
    FILE *file = file_open(path, STRING_VIEW_CSTR("r"));
    if (file == nullptr)
    {
        return EFILE;
    }
    u64 file_len = get_file_len(file);
    *result = string_create_with_size(file_len);
    // TODO: add assert
    fread(result->data, sizeof(char), result->size, file);

    return SUCCESS;
}
