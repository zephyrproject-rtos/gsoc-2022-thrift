# Thrift for Zephyr

This repository contains a Zephyr module to support Thrift.

## Getting Started

Before getting started, make sure you have a proper Zephyr development
environment. You can follow the official
[Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html).

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

If this is the first time building Zephyr, please set up your `~/.zephyrrc` file:

```shell
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
# Will vary based on the installed SDK version
export ZEPHYR_SDK_INSTALL_DIR=~/zephyr-sdk-0.13.1
```

```
source ~/.zephyrrc
```

### Build & Run the Hello Test

The application can be built by running:

```shell
west build -p auto -b qemu_x86_64 -t run -s ../thrift-for-zephyr/tests/lib/thrift/hello
```
