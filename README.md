# Thrift for Zephyr

This repository contains a Zephyr module to support Thrift.

## Build Status

[![Build](https://github.com/cfriedt/thrift-for-zephyr/actions/workflows/build.yml/badge.svg)](https://github.com/cfriedt/thrift-for-zephyr/actions)

## Getting Started

Before getting started, make sure you have a proper Zephyr development
environment by following the official
[Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html).

### Installing Thrift Binary
Aside from the dependencies in the Zephyr Getting Started Guide above, you will also need to install Thrift. On MacOS, you can use:

```brew install thrift```

### Initialization

The first step is to initialize the workspace folder (``~/my-workspace``) where
the ``thrift-for-zephyr`` and all Zephyr modules will be cloned. You can do
that by running:

```shell
export WS=~/my-workspace
# initialize my-workspace for thrift-for-zephyr (main branch)
west init -m https://github.com/cfriedt/thrift-for-zephyr --mr main ${WS}
# update Zephyr modules
cd ${WS}
west update
```

### One-time Setup

If this is the first time building Zephyr, please set up your `~/.zephyrrc` file:
```shell
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
# Will vary based on the installed SDK version
export ZEPHYR_SDK_INSTALL_DIR=~/zephyr-sdk-0.13.1
```

### Qemu Setup

When running Zephyr inside of Qemu, a UNIX domain socket is used as a virtual serial port.
Run this command in the background (with `&`). Later, the process can be stopped with
`fg` and  `Ctrl+C`.
```shell
${WS}/net-tools/loop-socat.sh &
```

Additionally, Qemu user-mode networking is used, and we forward incoming TCP/IP traffic
on port 4242 to the Qemu instance.


### Build & Run the Testsuite

Run the testsuite with:
```shell
cd ${WS}/thrift-for-zephyr
source zephyr-env.sh
west build -p auto -b qemu_x86_64 -t run tests/lib/thrift/hello
...
Booting from ROM..*** Booting Zephyr OS build zephyr-v3.0.0-1366-g1c66e53f7846  ***
Running test suite thrift_hello
===================================================================
START - test_hello
Starting the server...
ping
echo: Hello, world!
counter: 1
counter: 2
counter: 3
counter: 4
counter: 5
```
