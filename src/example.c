#include <stdio.h>
#include "shared/shared.h"
#include "shared/flags.h"
#include <event2/listener.h>
#include <event2/bufferevent.h>

typedef struct
{
    PPP_C_FLAGS_BOOL sync;
    PPP_C_FLAGS_STRING addr;
    PPP_C_FLAGS_UINT8 balance;
    PPP_C_FLAGS_UINT8 worker;
} root_flags_t;
static int root_handler(ppp_c_flags_command_t *command, int argc, char **argv, void *userdata)
{
    root_flags_t *flags = userdata;
    printf("sync=%s\naddr=%s\nbalance=%u\nworker=%u\n",
           flags->sync ? "true" : "false",
           flags->addr,
           flags->balance,
           flags->worker);
    return 0;
}
int main(int argc, char **argv)
{
    int err = 0;
    // create command
    root_flags_t flags = {
        .sync = 0,
        .addr = ":9000",
        .balance = LIBEVENT_HTTP_SERVE__BALANCE_ROUND,
        .worker = 8,
    };
    ppp_c_flags_command_t *cmd = ppp_c_flags_command_create(
        ppp_c_flags_base_name(argv[0]),
        "libevent_http_serve example",
        root_handler, &flags,
        &err);
    if (!cmd)
    {
        printf("Create command fail: %s\n", ppp_c_flags_error(err));
        return -1;
    }

    // add flags
    PPP_C_FLAGS_ADD_FLAG(
        cmd,
        "sync", 's',
        "Use synchronized listener in separate thread",
        &flags.sync, PPP_C_FLAGS_TYPE_BOOL,
        &err, FAIL)
    PPP_C_FLAGS_ADD_FLAG(
        cmd,
        "addr", 'a',
        "HTTP listen address, [host]:port, exmaple [::]:80 127.0.0.1:80.",
        &flags.addr, PPP_C_FLAGS_TYPE_STRING,
        &err, FAIL)
    PPP_C_FLAGS_ADD_FLAG(
        cmd,
        "balance", 'b',
        "Load balancing algorithm. 1(ROUND) 2(RANDOM)",
        &flags.balance, PPP_C_FLAGS_TYPE_UINT8,
        &err, FAIL)
    PPP_C_FLAGS_ADD_FLAG(
        cmd,
        "worker", 'w',
        "Number of worker threads",
        &flags.worker, PPP_C_FLAGS_TYPE_UINT8,
        &err, FAIL)

    // Parse and execute commands
    int handler_result = 0;
    err = ppp_c_flags_execute(
        cmd,
        argc - 1, argv + 1, // Remove program startup path
        1, &handler_result);
    if (handler_result && !err)
    {
        err = handler_result;
    }

FAIL:
    // Clean up resources used by commands
    ppp_c_flags_command_destroy(cmd);
    return err;
}
