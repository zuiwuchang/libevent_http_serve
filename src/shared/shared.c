#include "shared.h"
#include <string.h>
#include <stdio.h>
#include "flags.h"

int libevent_http_serve_parse_address(const char *s, size_t s_len, libevent_http_serve_address_t *addr)
{
    size_t i = s_len - 1;
    for (; i != -1; i--)
    {
        if (s[i] == ':')
        {
            break;
        }
    }
    if (i == -1)
    {
        return -1;
    }
    uint64_t port;
    if (ppp_c_flags_parse_uint64(s + i + 1, s_len - i - 1, 10, 8 * 2, &port) || !port)
    {
        return -1;
    }
    s_len = i;
    if (s_len == 0)
    {
        // is support ipv6
        struct sockaddr_in6 sin = {0};
        if (evutil_inet_pton(AF_INET6, "::", &sin.sin6_addr))
        {
            // v6 any
            if (addr)
            {
                addr->v6 = 1;
                memset(&addr->addr6, 0, sizeof(struct sockaddr_in6));
                addr->addr6.sin6_family = AF_INET6;
                addr->addr6.sin6_port = htons((uint16_t)port);
                addr->addr6.sin6_addr = in6addr_any;
            }
        }
        else
        {
            // v4 any
            if (addr)
            {
                addr->v6 = 0;
                memset(&addr->addr, 0, sizeof(struct sockaddr_in));
                addr->addr.sin_family = AF_INET;
                addr->addr.sin_port = htons((uint16_t)port);
                addr->addr.sin_addr.s_addr = INADDR_ANY;
            }
        }
        return 0;
    }
    else if (s_len < 2)
    {
        return -1;
    }
    else if (s[0] == '[' && s[s_len - 1] == ']')
    {
        // v6
        s_len -= 2;
        if (s_len > LIBEVENT_HTTP_SERVE__MAX_IPV6_STRING_LEN)
        {
            return -1;
        }
        s++;
        uint8_t ip[LIBEVENT_HTTP_SERVE__MAX_IPV6_STRING_LEN + 1];
        memcpy(ip, s, s_len);
        ip[s_len] = 0;

        if (addr)
        {
            addr->v6 = 1;
            memset(&addr->addr6, 0, sizeof(struct sockaddr_in6));
            if (evutil_inet_pton(AF_INET6, ip, &addr->addr6.sin6_addr))
            {
                return -1;
            }
            addr->addr6.sin6_family = AF_INET6;
            addr->addr6.sin6_port = htons((uint16_t)port);
        }
        else
        {
            struct sockaddr_in6 addr;
            if (evutil_inet_pton(AF_INET6, ip, &addr.sin6_addr))
            {
                return -1;
            }
        }
    }
    else
    {
        // v4
        if (s_len > LIBEVENT_HTTP_SERVE__MAX_IPV4_STRING_LEN)
        {
            return -1;
        }
        uint8_t ip[LIBEVENT_HTTP_SERVE__MAX_IPV4_STRING_LEN + 1];
        memcpy(ip, s, s_len);
        ip[s_len] = 0;

        if (addr)
        {
            memset(&addr->addr, 0, sizeof(struct sockaddr_in));
            if (evutil_inet_pton(AF_INET, ip, &addr->addr.sin_addr))
            {
                return -1;
            }
            addr->addr.sin_family = AF_INET;
            addr->addr.sin_port = htons((uint16_t)port);
        }
        else
        {
            struct sockaddr_in addr;
            if (evutil_inet_pton(AF_INET, ip, &addr.sin_addr))
            {
                return -1;
            }
        }
    }
    return 0;
}
static void libevent_http_serve_worker_on_ev(evutil_socket_t _, short events, void *arg)
{
    libevent_http_serve_worker_t *worker = arg;
    printf("id=%lu\n", worker->id);
}
static void *libevent_http_serve_worker_on_thread(void *arg)
{
    libevent_http_serve_worker_t *worker = arg;
    event_base_dispatch(worker->base);
    return 0;
}
int libevent_http_serve_worker_init(libevent_http_serve_worker_t *worker)
{
    if (pthread_mutex_init(&worker->mutex, 0))
    {
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
    worker->ev = event_new(worker->base, -1, EV_PERSIST, libevent_http_serve_worker_on_ev, worker);
    if (!worker->ev)
    {
        puts("event_new fail");
        goto FAIL;
    }
    struct timeval tv =
        {
            // .tv_sec = 3600 * 24 * 365,
            .tv_sec = 1,
            .tv_usec = 0,
        };
    if (event_add(worker->ev, &tv))
    {
        puts("event_add fail");
        goto FAIL;
    }

    if (pthread_create(&worker->thread, 0, libevent_http_serve_worker_on_thread, worker))
    {
        puts("pthread_create fail");
        goto FAIL;
    }
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
void libevent_http_serve_worker_destroy(libevent_http_serve_worker_t *worker)
{
}
int libevent_http_serve_load_balancer_init(libevent_http_serve_load_balancer_t *load_balancer)
{
    size_t n = sizeof(libevent_http_serve_worker_t) * load_balancer->worker;
    load_balancer->_workers = malloc(n);
    if (!load_balancer->_workers)
    {
        puts("malloc workers fail");
        return -1;
    }
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
            return -1;
        }
        (load_balancer->_workers + i)->id = i;
    }
    return 0;
}
void libevent_http_serve_load_balancer_destroy(libevent_http_serve_load_balancer_t *load_balancer)
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