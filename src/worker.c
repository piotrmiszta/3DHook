#include "worker.h"
#include "err_codes.h"
#include "http_parser.h"
#include "logger.h"
#include "str.h"
#include "utils.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>

constexpr u32 max_clients_in_que = 10;
static volatile bool worker_run = false;

typedef struct WorkerState
{
    u32 add_index;
    u32 get_index;
    pthread_mutex_t mtx;
    sem_t que_full;
    sem_t que_empty;
    Client *que[max_clients_in_que];
    pthread_t thread;
} WorkerState;

static WorkerState state;

static void *worker_thread(void *);

err_t worker_boot(void)
{
    worker_run = true;
    state.add_index = 0;
    state.get_index = 0;
    pthread_mutex_init(&state.mtx, NULL);
    sem_init(&state.que_empty, 0, max_clients_in_que);
    sem_init(&state.que_full, 0, 0);
    pthread_create(&state.thread, NULL, worker_thread, NULL);
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
    log_debug("Added client from que");
    return SUCCESS;
}

static str_view_t path_to_root = CONST_STRING_VIEW_CSTR("/var/www/html/");

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
    if (string_view_equal(&msg->url, &STRING_VIEW_CSTR("/")) == true)
    {
        printf("Path is root! \n");
        path = string_view_join(&path_to_root, &STRING_VIEW_CSTR("home.html"),
                                buffer, 256);
        string_view_fprintf(stdout, &path);
        printf("\n");
    }

    if (!path.data)
    {
        return EMEMORY;
    }

    str_t response_header =
        string_create_from_cstr("HTTP/1.1 200 OK\n"
                                "Content-Type: text/html\n");
    str_t file = get_file_to_memory(path);
    char content_len[100];
    sprintf(content_len, "Content-Length: %lu\n\n", file.size);
    str_t con_len = string_create_from_buff(strlen(content_len), content_len);
    string_join(&response_header, &con_len);
    string_join(&response_header, &file);

    *result = response_header;
    if (result->data == nullptr)
    {
        log_error("Cannot allocate memory for result message to client %d",
                  client_fd);
        return EMEMORY;
    }
    string_fprintf(stdout, result);
    string_free(&file);
    string_free(&con_len);
    return SUCCESS;
}

static void *worker_thread(void *)
{
    while (1)
    {
        sem_wait(&state.que_full);
        if (worker_run == false)
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
        log_debug("Popped client from que");
        HttpMessage msg;
        http_message_parse(&msg, req->message);

        str_t response;
        worker_process(&msg, req->socket, &response);
        pthread_mutex_lock(&req->mtx);
        req->reponse = response;
        req->response_ready = true;
        pthread_mutex_unlock(&req->mtx);
        http_message_free(&msg);
    }
    return NULL;
}

void worker_close(void)
{
    worker_run = false;
    sem_post(&state.que_full);
    pthread_join(state.thread, NULL);
}
