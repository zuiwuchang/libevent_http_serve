#ifndef LIBEVENT_HTTP_SERVE__SHARED_H
#define LIBEVENT_HTTP_SERVE__SHARED_H

#include <stdint.h>

#include <pthread.h>

#include <event2/thread.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/http.h>

#define LIBEVENT_HTTP_SERVE__BALANCE_ROUND 1
#define LIBEVENT_HTTP_SERVE__BALANCE_RANDOM 2

#define LIBEVENT_HTTP_SERVE__MAX_IPV4_STRING_LEN 15
#define LIBEVENT_HTTP_SERVE__MAX_IPV6_STRING_LEN 46

typedef struct
{
    uint8_t v6;
    union
    {
        struct sockaddr_in addr;
        struct sockaddr_in6 addr6;
    };
} libevent_http_serve_address_t;
int libevent_http_serve_parse_address(const char *s, size_t s_len, libevent_http_serve_address_t *addr);

typedef struct
{
    size_t id;

    pthread_mutex_t mutex;
    pthread_t thread;

    struct event_base *base;
    struct evhttp *http;
    struct event *ev;
} libevent_http_serve_worker_t;
int libevent_http_serve_worker_init(libevent_http_serve_worker_t *worker);
void libevent_http_serve_worker_destroy(libevent_http_serve_worker_t *worker);
typedef struct
{
    uint8_t balance;

    uint8_t worker;
    libevent_http_serve_worker_t *_workers;
} libevent_http_serve_load_balancer_t;

int libevent_http_serve_load_balancer_init(libevent_http_serve_load_balancer_t *load_balancer);
void libevent_http_serve_load_balancer_destroy(libevent_http_serve_load_balancer_t *load_balancer);
#endif