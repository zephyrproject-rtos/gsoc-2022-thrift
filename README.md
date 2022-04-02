# Thrift for Zephyr

This repository contains a Zephyr module to support Thrift.

## Build Status

[![Build](https://github.com/cfriedt/thrift-for-zephyr/actions/workflows/build.yml/badge.svg)](https://github.com/cfriedt/thrift-for-zephyr/actions)

## Getting Started

Before getting started, make sure you have a proper Zephyr development
environment by following the official
[Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html).

### Installing Thrift Binary

**MacOS**
```shell
brew install thrift
```

**Ubuntu**
```shell
apt install -y libboost-all-dev thrift-compiler libthrift-dev
```

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

For networked examples (Linux-only for now), run the following to build additional
network utilities.
```shell
cd ${WS}/net-tools
make
```

### Qemu Setup

When running Zephyr inside of Qemu, a UNIX domain socket is used as a virtual serial port.
Run this command in the background (with `&`). Later, the process can be stopped with
`fg` and  `Ctrl+C`.
```shell
${WS}/net-tools/loop-socat.sh &
```

For networked examples (Linux-only for now), do the following:
```
sudo ${WS}/net-tools/loop-slip-tap.sh
# Enter password when prompted
# press Ctrl+Z to background the process
```

Support for networked examples under macOS is a [work-in-progress](https://github.com/zephyrproject-rtos/zephyr/issues/15738). Thank you for your patience.


### Build & Run the Testsuite

Run the testsuite with:
```shell
cd ${WS}/thrift-for-zephyr
source zephyr-env.sh
west build -p auto -b qemu_x86_64 -t run tests/lib/thrift/hello
...
Booting from ROM..
SeaBIOS (version rel-1.15.0-0-g2dd4b9b3f840-prebuilt.qemu.org)
*** Booting Zephyr OS build zephyr-v3.0.0-1366-g1c66e53f7846  ***
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
Server is done
 PASS - test_hello in 0.18 seconds
===================================================================
Test suite thrift_hello succeeded
===================================================================
PROJECT EXECUTION SUCCESSFUL
```

### Build & Run the Hello Server Sample App

This only works in Linux, for now.

Run the `hello_server` sample application with:
```shell
cd ${WS}/thrift-for-zephyr
source zephyr-env.sh
west build -p auto -b qemu_x86_64 -t run samples/lib/thrift/hello_server
...
Booting from ROM..
SeaBIOS (version rel-1.15.0-0-g2dd4b9b3f840-prebuilt.qemu.org)
*** Booting Zephyr OS build zephyr-v3.0.0-1366-gca26ff490759  ***


[00:00:00.010,000] <inf> net_config: Initializing network
[00:00:00.010,000] <inf> net_config: IPv4 address: 192.0.2.1
[00:00:00.110,000] <inf> net_config: IPv6 address: 2001:db8::1
[00:00:00.110,000] <inf> net_config: IPv6 address: 2001:db8::1
uart:~$ 
```

Note, the `uart:~$` prompt is for the [Zephyr Shell](https://docs.zephyrproject.org/latest/reference/shell/index.html), which has a slew of really useful utilities! Use the autocomplete feature by pressing `TAB` to discover which commands are available.

From another terminal, build and run the `hello_client` sample app compiled for the host OS.

```shell
make -j -C samples/lib/thrift/hello_client
./samples/lib/thrift/hello_client/hello_client
make -j -C samples/lib/thrift/hello_client clean
```

You should observe the following in the original `hello_server` terminal:
```
ping
echo: Hello, world!
counter: 1
counter: 2
counter: 3
counter: 4
counter: 5
```

### Build & Run the Hello Client Sample App

This only works in Linux, for now.

First, from another terminal, build and run the `hello_server` sample app compiled for the host OS.

```shell
make -j -C samples/lib/thrift/hello_server
./samples/lib/thrift/hello_client/hello_server 0.0.0.0
```

Then, in annother terminal, run the `hello_client` sample application with:
```shell
cd ${WS}/thrift-for-zephyr
source zephyr-env.sh
west build -p auto -b qemu_x86_64 -t run samples/lib/thrift/hello_client \
    -DCONFIG_NET_CONFIG_NEED_IPV6=n \
    -DCONFIG_NET_CONFIG_MY_IPV4_ADDR=\"192.0.2.1\" \
    -DCONFIG_NET_CONFIG_PEER_IPV4_ADDR=\"192.0.2.2\"
...
Booting from ROM..
SeaBIOS (version rel-1.15.0-0-g2dd4b9b3f840-prebuilt.qemu.org)
*** Booting Zephyr OS build zephyr-v3.0.0-1366-gca26ff490759  ***


[00:00:00.010,000] <inf> net_config: Initializing network
[00:00:00.010,000] <inf> net_config: IPv4 address: 192.0.2.1
[00:00:00.110,000] <inf> net_config: IPv6 address: 2001:db8::1
[00:00:00.110,000] <inf> net_config: IPv6 address: 2001:db8::1
uart:~$ 
```

You should observe the following in the original `hello_server` terminal:
```
ping
echo: Hello, world!
counter: 1
counter: 2
counter: 3
counter: 4
counter: 5
```
