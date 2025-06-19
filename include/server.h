#ifndef SERVER_H_
#define SERVER_H_

#include <netinet/in.h>
#include <pthread.h>

#include "err_codes.h"

typedef struct Server
{
    int32_t socket;
    struct sockaddr_in addr;
    pthread_t thread;
}Server;

[[nodiscard("Function can fail, depend on platform")]]
err_t server_boot(Server server[static 1]);

#endif
