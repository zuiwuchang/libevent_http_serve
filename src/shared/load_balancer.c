#include "load_balancer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void http_handler(struct evhttp_request *req, void *ctx)
{
    worker_t *worker = ctx;
    struct evbuffer *reply = evbuffer_new();
    evbuffer_add_printf(reply, "serve by worker %lu\n", worker->id);
    evhttp_send_reply(req, 200, "Ok", reply);
    evbuffer_free(reply);
}
static void worker_on_ev(evutil_socket_t _, short events, void *arg)
{
    worker_t *worker = arg;
    struct http_connection *next;
    while (1)
    {
        pthread_mutex_lock(&worker->mutex);
        if (!worker->next)
        {
            pthread_mutex_unlock(&worker->mutex);
            return;
        }
        next = worker->next;
        worker->next = next->next;
        pthread_mutex_unlock(&worker->mutex);

        evhttp_serve(
            worker->http,
            next->fd,
            (struct sockaddr *)(((uint8_t *)next) + sizeof(struct http_connection)),
            next->addr_len,
            0);

        free(next);
    }
}
static void *worker_on_thread(void *arg)
{
    worker_t *worker = arg;
    event_base_dispatch(worker->base);
    return 0;
}
int libevent_http_serve_worker_init(worker_t *worker)
{
    if (pthread_mutex_init(&worker->mutex, 0))
    {
        puts("pthread_mutex_init fail");
        return -1;
    }

    worker->base = event_base_new();
    if (!worker->base)
    {
        puts("event_base_new fail");
        goto FAIL;
    }
    worker->http = evhttp_new(worker->base);
    if (!worker->http)
    {
        puts("evhttp_new fail");
        goto FAIL;
    }
    evhttp_set_gencb(worker->http, http_handler, worker);
    worker->ev = event_new(worker->base, -1, EV_PERSIST, worker_on_ev, worker);
    if (!worker->ev)
    {
        puts("event_new fail");
        goto FAIL;
    }
    struct timeval tv =
        {
            .tv_sec = 3600 * 24 * 365,
            .tv_usec = 0,
        };
    if (event_add(worker->ev, &tv))
    {
        puts("event_add fail");
        goto FAIL;
    }

    if (pthread_create(&worker->thread, 0, worker_on_thread, worker))
    {
        puts("pthread_create fail");
        goto FAIL;
    }
    pthread_detach(worker->thread);
    return 0;
FAIL:
    pthread_mutex_destroy(&worker->mutex);
    if (worker->ev)
    {
        event_free(worker->ev);
        worker->ev = 0;
    }
    if (worker->http)
    {
        evhttp_free(worker->http);
        worker->http = 0;
    }
    if (worker->base)
    {
        event_base_free(worker->base);
        worker->base = 0;
    }
    return -1;
}
void libevent_http_serve_worker_destroy(worker_t *worker)
{
    if (worker->base)
    {
        pthread_mutex_destroy(&worker->mutex);
        event_free(worker->ev);
        worker->ev = 0;

        evhttp_free(worker->http);
        worker->http = 0;

        event_base_free(worker->base);
        worker->base = 0;
    }
}
int load_balancer_init(load_balancer_t *load_balancer)
{
    // malloc workers memory
    size_t n = sizeof(worker_t) * load_balancer->worker;
    load_balancer->_workers = malloc(n);
    if (!load_balancer->_workers)
    {
        puts("malloc workers fail");
        return -1;
    }
    if (pthread_mutex_init(&load_balancer->_mutex, 0))
    {
        free(load_balancer->_workers);
        load_balancer->_workers = 0;
        puts("pthread_mutex_init fail");
        return -1;
    }

    // create work thread
    memset(load_balancer->_workers, 0, n);
    for (size_t i = 0; i < load_balancer->worker; i++)
    {
        if (libevent_http_serve_worker_init(load_balancer->_workers + i))
        {
            for (size_t j = 0; j < i; j++)
            {
                libevent_http_serve_worker_destroy(load_balancer->_workers + j);
            }
            free(load_balancer->_workers);
            load_balancer->_workers = 0;
            pthread_mutex_destroy(&load_balancer->_mutex);
            return -1;
        }
        (load_balancer->_workers + i)->id = i;
    }
    return 0;
}
void load_balancer_destroy(load_balancer_t *load_balancer)
{
    if (load_balancer->_workers)
    {
        for (size_t i = 0; i < load_balancer->worker; i++)
        {
            libevent_http_serve_worker_destroy(load_balancer->_workers + i);
        }
        free(load_balancer->_workers);
        load_balancer->_workers = 0;
    }
}
void load_balancer_serve(load_balancer_t *load_balancer, evutil_socket_t s, struct sockaddr *addr, socklen_t addr_len)
{
    // Return worker according to algorithm
    worker_t *worker;
    switch (load_balancer->balance)
    {
    case LOAD_BALANCE_ROUND:
        worker = load_balancer->_workers + load_balancer->_i;
        if (++load_balancer->_i == load_balancer->worker)
        {
            load_balancer->_i = 0;
        }
        break;
    // case LOAD_BALANCE_RANDOM:
    default:
        worker = load_balancer->_workers + (rand() % load_balancer->worker);
        break;
    }

    // Create a task
    struct http_connection *conn = malloc(sizeof(struct http_connection) + addr_len);
    conn->next = 0;
    conn->fd = s;
    conn->addr_len = addr_len;
    memcpy(((uint8_t *)conn) + sizeof(struct http_connection), addr, addr_len);

    // Push task to back
    pthread_mutex_lock(&worker->mutex);
    if (worker->next)
    {
        struct http_connection *next = worker->next;
        while (next->next)
        {
            next = next->next;
        }
        next->next = conn;
    }
    else
    {
        worker->next = conn;
    }
    pthread_mutex_unlock(&worker->mutex);
    // active thread
    event_active(worker->ev, 0, 0);
}