#ifndef WORKER_H_
#define WORKER_H_

#include "err_codes.h"
#include "http_parser.h"

typedef struct
{
    HttpMessage *msg;
    /* TODO: Replace with client struct after refactoring */
    s32 client_fd;
} WorkerRequest;

err_t worker_process(HttpMessage msg[static 1], s32 client_fd,
                     str_t result[static 1]);

#endif
