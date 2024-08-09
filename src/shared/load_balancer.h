#ifndef LIBEVENT_HTTP_SERVE__LOAD_BALANCER_H
#define LIBEVENT_HTTP_SERVE__LOAD_BALANCER_H

#define LOAD_BALANCE_ROUND 1
#define LOAD_BALANCE_RANDOM 2

#include <stdint.h>

#include <pthread.h>

#include <event2/thread.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/http.h>
#include <event2/buffer.h>

struct http_connection
{
    struct http_connection *next;
    evutil_socket_t fd;
    socklen_t addr_len;
};

// Worker thread
typedef struct
{
    size_t id;

    pthread_mutex_t mutex;
    pthread_t thread;

    struct event_base *base;
    struct evhttp *http;
    struct event *ev;

    struct http_connection *next;
} worker_t;
int worker_init(worker_t *worker);
void worker_destroy(worker_t *worker);

/**
 * Provide load balancing services for new connections
 */
typedef struct
{
    // algorithm to use
    uint8_t balance;
    // How many worker threads to create
    uint8_t worker;

    // thread sync lock
    pthread_mutex_t _mutex;
    // worker thread
    worker_t *_workers;
    uint8_t _i;
} load_balancer_t;

int load_balancer_init(load_balancer_t *load_balancer);
void load_balancer_destroy(load_balancer_t *load_balancer);
void load_balancer_serve(load_balancer_t *load_balancer, evutil_socket_t s, struct sockaddr *addr, socklen_t addr_len);

#endif
