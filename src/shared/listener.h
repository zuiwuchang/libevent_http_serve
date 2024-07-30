#ifndef LIBEVENT_HTTP_SERVE__LISTENER_H
#define LIBEVENT_HTTP_SERVE__LISTENER_H

#include "shared.h"

typedef struct
{
    evutil_socket_t s;
    uint8_t v6;
    uint8_t ok;
} libevent_http_serve_sync_listener_t;

int libevent_http_serve_sync_listener_init(libevent_http_serve_sync_listener_t *l, libevent_http_serve_address_t *addr);
void libevent_http_serve_sync_listener_serve(libevent_http_serve_sync_listener_t *l);

#endif