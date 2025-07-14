#ifndef SERVER_H_
#define SERVER_H_

#include <netinet/in.h>
#include <pthread.h>

#include "err_codes.h"
#include "list.h"
#include "str.h"
#include "types.h"

typedef struct Client
{
    s32 socket;
    struct sockaddr_in addr;
    list_t list;
    str_t message;
    str_t reponse;
    volatile bool response_ready;
    /* TODO: if we store here message and response,
       we need to make sure that nothing will replace this strings */
} Client;

typedef struct Server
{
    s32 socket;
    struct sockaddr_in addr;
    pthread_t thread;
    list_t clients;
} Server;

[[nodiscard("Function can fail, depend on platform")]]
err_t server_boot(Server server[static 1]);

err_t server_close(Server server[static 1]);

#endif
