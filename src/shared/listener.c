#include "listener.h"
#include <errno.h>
#include <string.h>
int libevent_http_serve_sync_listener_init(libevent_http_serve_sync_listener_t *l, libevent_http_serve_address_t *addr)
{
    evutil_socket_t s = socket(addr->v6 ? AF_INET6 : AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == -1)
    {
        printf("socket fail: %d %s", errno, strerror(errno));
        return -1;
    }
    int err = addr->v6 ? bind(s, (const struct sockaddr *)&addr->addr6, sizeof(struct sockaddr_in6)) : bind(s, (const struct sockaddr *)&addr->addr, sizeof(struct sockaddr_in));
    if (err < 0)
    {
        evutil_closesocket(s);
        return -1;
    }
    if (listen(s, 128) < 0)
    {
        evutil_closesocket(s);
        return -1;
    }

    l->s = s;
    l->v6 = addr->v6;
    l->ok = 1;
    return 0;
}

static void libevent_http_serve_sync_listener_serve_impl(libevent_http_serve_sync_listener_t *l, struct sockaddr *addr, socklen_t addr_len)
{
    evutil_socket_t s;
    while (l->ok)
    {
        s = accept(l->s, addr, &addr_len);
        if (s == -1)
        {
            printf("accept fail: %d %s", errno, strerror(errno));
            continue;
        }
    }
}
void libevent_http_serve_sync_listener_serve(libevent_http_serve_sync_listener_t *l)
{
    if (l->v6)
    {
        struct sockaddr_in6 addr;
        libevent_http_serve_sync_listener_serve_impl(l, (struct sockaddr *)&addr, sizeof(struct sockaddr_in6));
    }
    else
    {
        struct sockaddr_in addr;
        libevent_http_serve_sync_listener_serve_impl(l, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    }
}