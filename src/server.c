#include "server.h"
#include "err_codes.h"
#include "http_parser.h"
#include "list.h"
#include "logger.h"
#include "str.h"
#include "worker.h"
#include <bits/pthreadtypes.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

constexpr s32 port = 8080;
constexpr s32 epoll_max_file_descrptors = 1000;
constexpr s32 epoll_max_events = 100;

static volatile bool server_run = false;

static void *server_thread(void *arg);
static err_t server_set_noblock(s32 fd);
static err_t server_add_event(s32 epoll, s32 fd, u32 state);
static err_t server_read(Client *client);
static err_t server_write(Client *client);
static err_t server_handle_events(s32 epoll, struct epoll_event *events,
                                  s32 events_count, Server *server);

err_t server_boot(Server server[restrict static 1])
{
    server_run = true;
    worker_boot();
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
    list_init(&server->clients);

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

    s32 epoll = epoll_create(epoll_max_file_descrptors);
    struct epoll_event events[epoll_max_events];
    Server *server = arg;
    server_set_noblock(server->socket);
    server_add_event(epoll, server->socket, EPOLLIN);
    while (server_run)
    {
        s32 no_events = epoll_wait(epoll, events, epoll_max_events, 100);
        server_handle_events(epoll, events, no_events, server);
    }
    return NULL;
}

static err_t server_set_noblock(s32 fd)
{
    s32 flags = fcntl(fd, F_GETFL, 0);
    ASSERT(flags >= 0, "Cannot get flags from FD"); // TODO: error handling
    flags |= O_NONBLOCK;
    auto res = fcntl(fd, F_SETFL, flags);
    ASSERT(res >= 0, "Cannot set flags for FD");
    return SUCCESS;
}

static err_t server_add_event(s32 epoll, s32 fd, u32 state)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = state;
    auto res = epoll_ctl(epoll, EPOLL_CTL_ADD, fd, &ev);
    ASSERT(res >= 0, "Cannot add event for FD");
    return SUCCESS;
}

static err_t server_accept_client(s32 epoll, Server *server)
{
    Client *client = calloc(1, sizeof(Client));
    ASSERT(client, "Cannot alloc!");
    socklen_t len = 0;
    client->socket =
        accept(server->socket, (struct sockaddr *)&client->addr, &len);
    ASSERT(client->socket > 0, "Cannot accept client!");
    log_trace("Accepted new client %d", client->socket);
    pthread_mutex_init(&client->mtx, NULL);
    list_add_tail(&client->list, &server->clients);
    server_add_event(epoll, client->socket, EPOLLIN);
    return SUCCESS;
}

static err_t server_remove_client(Client *client)
{
    list_rem_entry(&client->list);
    log_trace("Disconnected client: %d", client->socket);
    free(client);
    return SUCCESS;
}

static err_t server_read(Client *client)
{
    char buffer[1024] = {};
    ssize_t readed = read(client->socket, buffer, 1024);
    if (readed <= 0)
    {
        server_remove_client(client);
        return EGENRIC;
    }
    str_t message = string_create_from_buff(readed - 1, buffer);
    pthread_mutex_lock(&client->mtx);
    client->message = message;
    pthread_mutex_unlock(&client->mtx);
    worker_add_request(client);
    log_debug("Writed message to client %d", client->socket);
    return SUCCESS;
}

static err_t server_write(Client *client)
{
    pthread_mutex_lock(&client->mtx);
    write(client->socket, client->reponse.data, client->reponse.size);
    client->response_ready = false;
    string_free(&client->reponse);
    pthread_mutex_unlock(&client->mtx);
    log_debug("Writed message to client %d", client->socket);
    return SUCCESS;
}

static err_t server_handle_events(s32 epoll, struct epoll_event *events,
                                  s32 events_count, Server *server)
{
    for (s32 i = 0; i < events_count; i++)
    {
        s32 fd = events[i].data.fd;
        if (fd == server->socket)
        {
            server_accept_client(epoll, server);
        }
        else
        {
            list_t *list = server->clients.next;
            Client *c = NULL;
            while (list != &server->clients)
            {
                Client *f = list_get_ptr(list, Client, list);
                if (f->socket == fd)
                {
                    c = f;
                    break;
                }
                list = list->next;
            }
            ASSERT(c != nullptr,
                   "Client not found in list, but was in epoll!!");
            if (events[i].events & EPOLLIN)
            {
                if (server_read(c) == EGENRIC)
                {
                    epoll_ctl(epoll, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                    continue;
                }

                struct epoll_event ev;
                ev.events = EPOLLOUT;
                ev.data.fd = fd;
                epoll_ctl(epoll, EPOLL_CTL_MOD, fd, &ev);
            }
            else if (events[i].events & EPOLLOUT && c->response_ready)
            {
                server_write(c);
                struct epoll_event ev;
                ev.events = EPOLLIN;
                ev.data.fd = fd;
                epoll_ctl(epoll, EPOLL_CTL_MOD, fd, &ev);
            }
        }
    }
    return SUCCESS;
}

err_t server_close(Server server[static 1])
{
    server_run = false;
    pthread_join(server->thread, NULL);
    list_t *clients = server->clients.next;
    while (clients != &server->clients)
    {
        Client *cl = list_get_ptr(clients, Client, list);
        clients = clients->next;
        if (cl->reponse.data)
        {
            string_free(&cl->reponse);
        }

        close(cl->socket);
        free(cl);
    }
    return SUCCESS;
}

static err_t server_set_noblock(s32 fd)
{
    s32 flags = fcntl(fd, F_GETFL, 0);
    ASSERT(flags >= 0, "Cannot get flags from FD"); // TODO: error handling
    flags |= O_NONBLOCK;
    auto res = fcntl(fd, F_SETFL, flags);
    ASSERT(res >= 0, "Cannot set flags for FD");
    return SUCCESS;
}

static err_t server_add_event(s32 epoll, s32 fd, u32 state)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = state;
    auto res = epoll_ctl(epoll, EPOLL_CTL_ADD, fd, &ev);
    ASSERT(res >= 0, "Cannot add event for FD");
    return SUCCESS;
}

static err_t server_accept_client(s32 epoll, Server *server)
{
    Client *client = calloc(1, sizeof(Client));
    ASSERT(client, "Cannot alloc!");
    socklen_t len = 0;
    client->socket =
        accept(server->socket, (struct sockaddr *)&client->addr, &len);
    ASSERT(client->socket > 0, "Cannot accept client!");
    log_trace("Accepted new client %d", client->socket);
    list_add_tail(&client->list, &server->clients);
    server_add_event(epoll, client->socket, EPOLLIN);
    return SUCCESS;
}

static err_t server_remove_client(Client *client)
{
    list_rem_entry(&client->list);
    log_trace("Disconnected client: %d", client->socket);
    free(client);
    return SUCCESS;
}

static err_t server_read(Client *client)
{
    char buffer[1024] = {};
    ssize_t readed = read(client->socket, buffer, 1024);
    if (readed <= 0)
    {
        server_remove_client(client);
        return EGENRIC;
    }
    str_t message = string_create_from_buff(readed - 1, buffer);
    pthread_mutex_lock(&client->mtx);
    client->message = message;
    pthread_mutex_unlock(&client->mtx);
    worker_add_request(client);
    string_fprintf(stdout, &message);
    return SUCCESS;
}

static err_t server_write(Client *client)
{
    pthread_mutex_lock(&client->mtx);
    write(client->socket, client->reponse.data, client->reponse.size);
    client->response_ready = false;
    string_free(&client->reponse);
    pthread_mutex_unlock(&client->mtx);
    log_debug("Writed message to client %d", client->socket);
    return SUCCESS;
}

static err_t server_handle_events(s32 epoll, struct epoll_event *events,
                                  s32 events_count, Server *server)
{
    for (s32 i = 0; i < events_count; i++)
    {
        s32 fd = events[i].data.fd;
        if (fd == server->socket)
        {
            server_accept_client(epoll, server);
        }
        else
        {
            list_t *list = server->clients.next;
            Client *c = NULL;
            while (list != &server->clients)
            {
                Client *f = list_get_ptr(list, Client, list);
                if (f->socket == fd)
                {
                    c = f;
                    break;
                }
                list = list->next;
            }
            ASSERT(c != nullptr,
                   "Client not found in list, but was in epoll!!");
            if (events[i].events & EPOLLIN)
            {
                if (server_read(c) == EGENRIC)
                {
                    epoll_ctl(epoll, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                    continue;
                }

                struct epoll_event ev;
                ev.events = EPOLLOUT;
                ev.data.fd = fd;
                epoll_ctl(epoll, EPOLL_CTL_MOD, fd, &ev);
            }
            else if (events[i].events & EPOLLOUT && c->response_ready)
            {
                server_write(c);
                struct epoll_event ev;
                ev.events = EPOLLIN;
                ev.data.fd = fd;
                epoll_ctl(epoll, EPOLL_CTL_MOD, fd, &ev);
            }
        }
    }
    return SUCCESS;
}

err_t server_close(Server server[static 1])
{
    server_run = false;
    pthread_join(server->thread, NULL);
    list_t *clients = server->clients.next;
    while (clients != &server->clients)
    {
        Client *cl = list_get_ptr(clients, Client, list);
        clients = clients->next;
        if (cl->reponse.data)
        {
            string_free(&cl->reponse);
        }

        close(cl->socket);
        free(cl);
    }
    return SUCCESS;
}
