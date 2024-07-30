#include <stdio.h>
#include <string.h>
#include "shared/shared.h"
#include "shared/listener.h"
#include "shared/flags.h"
// Define command line flags and automatic verification functions
typedef struct
{
    PPP_C_FLAGS_BOOL sync;
    PPP_C_FLAGS_STRING addr;
    PPP_C_FLAGS_UINT8 balance;
    PPP_C_FLAGS_UINT8 worker;

} root_flags_t;
static int verify_addr(struct ppp_c_flags_flag *flag, uint8_t value_type, void *old_value, void *new_value)
{
    const char *s = *(PPP_C_FLAGS_STRING *)new_value;
    return libevent_http_serve_parse_address(s, strlen(s), 0);
}
static int verify_balance(struct ppp_c_flags_flag *flag, uint8_t value_type, void *old_value, void *new_value)
{
    switch (*(PPP_C_FLAGS_UINT8 *)new_value)
    {
    case LIBEVENT_HTTP_SERVE__BALANCE_ROUND:
    case LIBEVENT_HTTP_SERVE__BALANCE_RANDOM:
        return 0;
    }
    return -1;
}
static int verify_worker(struct ppp_c_flags_flag *flag, uint8_t value_type, void *old_value, void *new_value)
{
    if (*(PPP_C_FLAGS_UINT8 *)new_value < 2)
    {
        return -1;
    }
    return 0;
}

// Processing function after parsing the command line
static int root_handler(ppp_c_flags_command_t *command, int argc, char **argv, void *userdata)
{
    // get flags
    root_flags_t *flags = userdata;
    // parse address
    libevent_http_serve_address_t addr;
    libevent_http_serve_parse_address(flags->addr, strlen(flags->addr), &addr);

    // enable multithreading
    if (evthread_use_pthreads())
    {
        puts("evthread_use_pthreads fail");
        return -1;
    }

    printf("sync=%s\naddr=%s\nbalance=%u\nworker=%u\n",
           flags->sync ? "true" : "false",
           flags->addr,
           flags->balance,
           flags->worker);

    // Create a load balancer to distribute requests to different threads for processing
    libevent_http_serve_load_balancer_t load_balancer = {
        .balance = flags->balance, // algorithm
        .worker = flags->worker,   // how many threads to start
    };
    if (libevent_http_serve_load_balancer_init(&load_balancer))
    {
        return -1;
    }

    int err = 0;
    libevent_http_serve_sync_listener_t l = {0};
    if (err = libevent_http_serve_sync_listener_init(&l, &addr))
    {
        goto END;
    }
    printf("http sync listener work on: %s\n", flags->addr);
    libevent_http_serve_sync_listener_serve(&l);

// if (flags->sync)
// {

// }
END:
    // free all resources
    libevent_http_serve_load_balancer_destroy(&load_balancer);
    libevent_global_shutdown();
    return err;
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
    ppp_c_flags_flag_t *flag;
    PPP_C_FLAGS_ADD_FLAG(
        cmd,
        "sync", 's',
        "Use synchronized listener in separate thread",
        &flags.sync, PPP_C_FLAGS_TYPE_BOOL,
        &err, FAIL)

    PPP_C_FLAGS_SET_ADD_FLAG(
        flag,
        cmd,
        "addr", 'a',
        "HTTP listen address, [host]:port, exmaple [::]:80 127.0.0.1:80.",
        &flags.addr, PPP_C_FLAGS_TYPE_STRING,
        &err, FAIL)
    flag->verify = verify_addr;

    PPP_C_FLAGS_SET_ADD_FLAG(
        flag,
        cmd,
        "balance", 'b',
        "Load balancing algorithm. 1(ROUND) 2(RANDOM)",
        &flags.balance, PPP_C_FLAGS_TYPE_UINT8,
        &err, FAIL)
    flag->verify = verify_balance;

    PPP_C_FLAGS_SET_ADD_FLAG(
        flag,
        cmd,
        "worker", 'w',
        "Number of worker threads",
        &flags.worker, PPP_C_FLAGS_TYPE_UINT8,
        &err, FAIL)
    flag->verify = verify_worker;

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
