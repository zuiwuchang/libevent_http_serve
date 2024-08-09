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

# bin

```
/example  -h
libevent_http_serve example

Usage:
  example [flags]

Flags:
  -s, --sync bool       Use synchronized listener in separate thread
  -a, --addr string     HTTP listen address, [host]:port, exmaple [::]:80 127.0.0.1:80. <default: :9000>
  -b, --balance uint8   Load balancing algorithm. 1(ROUND) 2(RANDOM) <default: 1>
  -w, --worker uint8    Number of worker threads <default: 8>
  -h, --help bool       Help for example
```

```
curl http://127.0.0.1:9000
```