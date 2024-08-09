#ifndef LIBEVENT_HTTP_SERVE__LISTENER_H
#define LIBEVENT_HTTP_SERVE__LISTENER_H

#include "load_balancer.h"
#include "shared.h"

/**
 *
 */
typedef struct
{
    load_balancer_t *load_balancer;
    evutil_socket_t s;
    uint8_t v6 : 1;
    uint8_t ok : 1;
} sync_listener_t;

int sync_listener_init(sync_listener_t *l, shared_address_t *addr, load_balancer_t *load_balancer);
void sync_listener_serve(sync_listener_t *l);

typedef struct
{
    load_balancer_t *load_balancer;
    struct event_base *base;
    struct evconnlistener *listener;
} async_listener_t;
int async_listener_init(async_listener_t *l, shared_address_t *addr, load_balancer_t *load_balancer);
void async_listener_serve(async_listener_t *l);

#endif