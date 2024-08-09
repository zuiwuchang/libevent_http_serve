#ifndef LIBEVENT_HTTP_SERVE__LISTENER_H
#define LIBEVENT_HTTP_SERVE__LISTENER_H

#include "load_balancer.h"
#include "shared.h"

/**
 * It receives socket connections using the synchronous API in a separate thread,
 * this demonstrates how to use a listener external to libevent with evhttp_serve
 */
typedef struct
{
    load_balancer_t *load_balancer;
    evutil_socket_t s;
    uint8_t v6 : 1;
    uint8_t ok : 1;
} sync_listener_t;
/**
 * Initialize listener
 */
int sync_listener_init(sync_listener_t *l, shared_address_t *addr, load_balancer_t *load_balancer);
/**
 * Let the listener execute the service
 */
void sync_listener_serve(sync_listener_t *l);

/**
 * It demonstrates how to use evhttp_serve using the evconnlistener provided by libevent.
 * Using evconnlistener allows the listener to work together with other libenevt asynchronous operations.
 */
typedef struct
{
    load_balancer_t *load_balancer;
    struct event_base *base;
    struct evconnlistener *listener;
} async_listener_t;
/**
 * Initialize listener
 */
int async_listener_init(async_listener_t *l, shared_address_t *addr, load_balancer_t *load_balancer);
/**
 * Let the listener execute the service
 */
void async_listener_serve(async_listener_t *l);

#endif