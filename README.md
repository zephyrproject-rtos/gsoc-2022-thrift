# Thrift for Zephyr

This repository contains a Zephyr module to support Thrift.

## Build Status

[![Build](https://github.com/cfriedt/thrift-for-zephyr/actions/workflows/build.yml/badge.svg)](https://github.com/cfriedt/thrift-for-zephyr/actions)

## Getting Started

Before getting started, make sure you have a proper Zephyr development
environment. You can follow the official
[Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html).

### Installing Thrift Binary
Aside from the dependencies in the Zephyr Getting Started Guide above, you will also need to install Thrift. On MacOS, you can use:

```brew install thrift```

### Initialization

The first step is to initialize the workspace folder (``my-workspace``) where
the ``thrift-for-zephyr`` and all Zephyr modules will be cloned. You can do
that by running:

```shell
# initialize my-workspace for thrift-for-zephyr (main branch)
west init -m https://github.com/cfriedt/thrift-for-zephyr --mr main my-workspace
# update Zephyr modules
cd my-workspace
west update
```

### One-time Setup

If this is the first time building the module, please set up your `~/.zephyrrc` file:
```shell
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
# Will vary based on the installed SDK version
export ZEPHYR_SDK_INSTALL_DIR=~/zephyr-sdk-0.13.1
```

### Create a Unix socket at /tmp/slip.sock

Zephyr expects this socket to be open for Qemu to communicate over as an emulated serial interface. Since Zephyr [net-tools](https://docs.zephyrproject.org/latest/guides/networking/qemu_setup.html#prerequisites) only supports Linux currently, we provide a utility for this so that you can also test on MacOS. In a separate terminal run:
```./tools/create_serial_socket.py```

### Build & Run the Hello Test App

Back in your main terminal, the application can be built by running:
```shell
cd my-workspace/thrift-for-zephyr
source ~/.zephyrrc
export ZEPHYR_BASE=${PWD}/../zephyr
west build -p auto -b qemu_x86_64 -t run tests/lib/thrift/hello
```
