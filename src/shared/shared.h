#ifndef LIBEVENT_HTTP_SERVE__SHARED_H
#define LIBEVENT_HTTP_SERVE__SHARED_H

#include <stdint.h>

#include <pthread.h>

#include <event2/thread.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/http.h>
#include <event2/buffer.h>

#define MAX_IPV4_STRING_LEN 15
#define MAX_IPV6_STRING_LEN 46

typedef struct
{
    uint8_t v6;
    union
    {
        struct sockaddr_in addr;
        struct sockaddr_in6 addr6;
    };
} shared_address_t;
int parse_shared_address(const char *s, size_t s_len, shared_address_t *addr);

#endif