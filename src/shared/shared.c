#include "shared.h"
#include <string.h>
#include <stdio.h>
#include "flags.h"

int parse_shared_address(const char *s, size_t s_len, shared_address_t *addr)
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
        if (s_len > MAX_IPV6_STRING_LEN)
        {
            return -1;
        }
        s++;
        uint8_t ip[MAX_IPV6_STRING_LEN + 1];
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
        if (s_len > MAX_IPV4_STRING_LEN)
        {
            return -1;
        }
        uint8_t ip[MAX_IPV4_STRING_LEN + 1];
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
