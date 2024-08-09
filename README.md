# libevent_http_serve

This project demonstrates how to use the http_serve function in libevent.

Because the official libevent project has not yet incorporated the http_serve function, this project does not import the official library but imports libevent from https://github.com/powerpuffpenguin/libevent.

# Build

This project uses [xmake](https://github.com/xmake-io/xmake) for construction. After installing the [xmake](https://github.com/xmake-io/xmake?tab=readme-ov-file#installation) environment, execute the xmake command to build.

```
xmake
```

# Special Note

In order to strive for simplicity, the project only provides an http server and returns a hello world message when you request it.

The project is only to demonstrate the core code, so it is only tested under Linux. Also, only the core code is commented.

# Source structure

| source | note |
|---|---|
|   src/example.c   |   Program entry point main    |
|   src/shared/flags.c |   Command line parsing library copied from https://github.com/zuiwuchang/c-flags  |
|   src/shared/load_balancer.c  | Implemented simple load balancing for calling evhttp_serve  |
|   src/shared/listener.c   |   Demonstrates how listener cooperates with load balancing    |
|   src/shared/shared.c |   Some other helper functions |


