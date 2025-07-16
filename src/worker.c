#include "worker.h"
#include "err_codes.h"
#include "http_gen.h"
#include "http_parser.h"
#include "logger.h"
#include "str.h"
#include "utils.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

constexpr u32 max_clients_in_que = 100;
constexpr u32 max_workers = 6;
static volatile bool worker_run[max_workers] = {0};
static pthread_t thread[max_workers];

typedef struct WorkerState
{
    u32 add_index;
    u32 get_index;
    pthread_mutex_t mtx;
    sem_t que_full;
    sem_t que_empty;
    Client *que[max_clients_in_que];
} WorkerState;

static WorkerState state;
static void *worker_thread(void *);

u32 *th_nm;
err_t worker_boot(void)
{
    th_nm = malloc(sizeof(u32) * max_workers);
    for (int32_t i = 0; i < max_workers; i++)
    {
        th_nm[i] = i;
        worker_run[i] = true;
        pthread_create(&thread[i], NULL, worker_thread, &th_nm[i]);
    }

    state.add_index = 0;
    state.get_index = 0;
    pthread_mutex_init(&state.mtx, NULL);
    sem_init(&state.que_empty, 0, max_clients_in_que);
    sem_init(&state.que_full, 0, 0);

    return SUCCESS;
}

err_t worker_add_request(Client client[static 1])
{
    sem_wait(&state.que_empty);
    pthread_mutex_lock(&state.mtx);
    state.que[state.add_index] = client;
    state.add_index++;
    if (state.add_index == max_clients_in_que)
    {
        state.add_index = 0;
    }
    pthread_mutex_unlock(&state.mtx);
    sem_post(&state.que_full);

    return SUCCESS;
}

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
    str_view_t path = msg->url;

    if (!path.data)
    {
        return EMEMORY;
    }

    str_t html = http_gen_get_page(path);
    if (html.data == nullptr)
    {
        return EGENRIC;
    }

    str_t response_header =
        string_create_from_cstr("HTTP/1.1 200 OK\n"
                                "Content-Type: text/html\n");
    char content_len[100];
    sprintf(content_len, "Content-Length: %lu\n\n", html.size);
    str_t con_len = string_create_from_buff(strlen(content_len), content_len);
    string_join(&response_header, &con_len);
    string_join(&response_header, &html);
    string_free(&html);
    *result = response_header;
    if (result->data == nullptr)
    {
        log_error("Cannot allocate memory for result message to client %d\n",
                  client_fd);
        return EMEMORY;
    }
    string_free(&con_len);
    return SUCCESS;
}

static void *worker_thread(void *arg)
{
    u32 *num = arg;
    u32 it = *num;
    while (1)
    {
        sem_wait(&state.que_full);
        if (worker_run[it] == false)
        {
            return NULL;
        }
        pthread_mutex_lock(&state.mtx);
        Client *req = state.que[state.get_index];
        state.get_index++;
        if (state.get_index == max_clients_in_que)
        {
            state.get_index = 0;
        }
        pthread_mutex_unlock(&state.mtx);
        sem_post(&state.que_empty);
        log_debug("Get %d client for %d worker\n", req->socket, it);
        HttpMessage msg;
        if (http_message_parse(&msg, req->message) != SUCCESS)
        {
            log_warning("Received unknown message!\n");
            continue;
        }

        str_t response;
        auto err = worker_process(&msg, req->socket, &response);
        http_message_free(&msg);
        if (err != SUCCESS)
        {
            continue;
        }

        pthread_mutex_lock(&req->mtx);
        req->reponse = response;
        req->response_ready = true;
        pthread_mutex_unlock(&req->mtx);
    }
    return NULL;
}

void worker_close(void)
{
    for (int32_t i = 0; i < max_workers; i++)
    {
        worker_run[i] = false;
    }
    for (int32_t i = 0; i < max_workers; i++)
    {
        sem_post(&state.que_full);
    }
    for (int32_t i = 0; i < max_workers; i++)
    {
        pthread_join(thread[i], NULL);
    }
    free(th_nm);
}
