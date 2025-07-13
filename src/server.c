#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "err_codes.h"
#include "http_parser.h"
#include "logger.h"
#include "server.h"
#include "str.h"
#include "worker.h"

constexpr int32_t port = 8080;

static void *server_thread(void *arg);

err_t server_boot(Server server[restrict static 1])
{
    int32_t fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd <= 0)
    {
        log_error("Can't create socket! %s", error());
        return ESOCKET;
    }

    struct sockaddr_in addr = {
        .sin_addr.s_addr = INADDR_ANY,
        .sin_family = AF_INET,
        .sin_port = htons(port),
    };
    socklen_t len = sizeof(addr);
    int32_t opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        log_error("Cannot apply options to socket! %s", error());
        return ESOCKET;
    }

    if (bind(fd, (struct sockaddr *)&addr, len) < 0)
    {
        log_error("Error when binding socket %s", error());
        return ESOCKET;
    }

    if (listen(fd, (int)3) < 0)
    {
        log_error("Error when listening! %s", error());
        return ESOCKET;
    }

    server->addr = addr;
    server->socket = fd;

    if (pthread_create(&server->thread, nullptr, server_thread, server) < 0)
    {
        log_error("Cannot create thread for server %s", error());
        return ESOCKET;
    }
    return SUCCESS;
}

static void *server_thread(void *arg)
{
    if (arg == nullptr)
    {
        log_error("pointer passed to server_thread is nullptr!");
        return nullptr;
    }
    Server *server = arg;
    struct sockaddr client_addr;
    socklen_t len = {};
    int32_t client_fd = accept(server->socket, &client_addr, &len);
    if (client_fd <= 0)
    {
        return nullptr;
    }
    log_info("Accepted client= %d", client_fd);

    int writed = 0;
    while (1)
    {
        char buffer[1024] = {};
        ssize_t readed = read(client_fd, buffer, 1024);
        HttpMessage msg;
        str_t message = string_create_from_buff(readed - 1, buffer);
        http_message_parse(&msg, message);
        str_t response;
        worker_process(&msg, client_fd, &response);
        printf("Sending: ");
        string_fprintf(stdout, &response);
        printf("\n");
        if (writed == 0)
        {
            write(client_fd, response.data, response.size);
            write(client_fd, "\0", 1);
            writed = 1;
        }
    }
}
