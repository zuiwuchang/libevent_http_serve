#include "listener.h"
#include <errno.h>
#include <string.h>
int sync_listener_init(sync_listener_t *l, shared_address_t *addr, load_balancer_t *load_balancer)
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

    l->load_balancer = load_balancer;
    l->s = s;
    l->v6 = addr->v6 ? 1 : 0;
    l->ok = 1;
    return 0;
}

static void sync_listener_serve_impl(sync_listener_t *l, struct sockaddr *addr, socklen_t addr_len)
{
    evutil_socket_t s;
    socklen_t len;
    while (l->ok)
    {
        len = addr_len;
        s = accept(l->s, addr, &len);
        if (s == -1)
        {
            printf("accept fail: %d %s", errno, strerror(errno));
            continue;
        }
        load_balancer_serve(l->load_balancer, s, addr, len);
    }
}
void sync_listener_serve(sync_listener_t *l)
{
    if (l->v6)
    {
        struct sockaddr_in6 addr;
        sync_listener_serve_impl(l, (struct sockaddr *)&addr, sizeof(struct sockaddr_in6));
    }
    else
    {
        struct sockaddr_in addr;
        sync_listener_serve_impl(l, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    }
}

int async_listener_init(async_listener_t *l, shared_address_t *addr, load_balancer_t *load_balancer)
{
    return 0;
}
void async_listener_serve(async_listener_t *l)
{
}
