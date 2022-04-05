# Thrift for Zephyr

<table>
<tr>
<td><img src="https://github.com/apache/thrift/raw/master/doc/images/thrift-layers.png" height="200"/></td>
<td><img src="https://upload.wikimedia.org/wikipedia/commons/thumb/6/64/Zephyr_RTOS_logo_2015.svg/640px-Zephyr_RTOS_logo_2015.svg.png" height="200"/></td>
<td><img src="https://upload.wikimedia.org/wikipedia/commons/thumb/7/7c/Google_Summer_of_Code_sun_logo_2022.svg/480px-Google_Summer_of_Code_sun_logo_2022.svg.png" height="200"/></td>
</tr>
</table>

[Thrift](https://github.com/apache/thrift) is an [IDL](https://en.wikipedia.org/wiki/Interface_description_language) specification, [RPC](https://en.wikipedia.org/wiki/Remote_procedure_call) framework, and [code generator](https://en.wikipedia.org/wiki/Automatic_programming). It works across all major operating systems, supports over 27 programming languages, 7 protocols, and 6 low-level transports. Thrift was originally developed at [Facebook in 2007](https://thrift.apache.org/static/files/thrift-20070401.pdf). Subsequently, it was donated to the [Apache Software Foundation](https://www.apache.org/). Thrift supports a rich set of types and data structures, and abstracts away transport and protocol details, which lets developers focus on application logic.

The [Zephyr Real-Time Operating System](https://zephyrproject.org/) was [adopted by The Linux Foundation in 2016](https://en.wikipedia.org/wiki/Zephyr_(operating_system)). It is mainly written in [C](https://en.wikipedia.org/wiki/C_(programming_language)), with tooling in [Python](https://www.python.org/) and [CMake](https://cmake.org/). Zephyr is configured using [Kconfig](https://www.kernel.org/doc/html/latest/kbuild/kconfig-language.html) and [DeviceTree](https://www.devicetree.org/) much like [the Linux kernel](https://en.wikipedia.org/wiki/Linux). With a strong focus on [IoT](https://en.wikipedia.org/wiki/Internet_of_things), Zephyr has first-class support for a wide array of different network protocols and libraries. Zephyr supports IP networking over a number of physical layers ranging from [IEEE 802.15.4](https://en.wikipedia.org/wiki/IEEE_802.15.4) and [BLE](https://en.wikipedia.org/wiki/Bluetooth_Low_Energy) to [CAN](https://en.wikipedia.org/wiki/CAN_bus), [WiFi](https://en.wikipedia.org/wiki/Wi-Fi), and [Ethernet](https://en.wikipedia.org/wiki/Ethernet). Although built for IoT, Zephyr scales from tiny Xtensa cores with 4kB of SRAM to AArch64 servers with hundreds of cores and GiB of DDR SDRAM.

## Google Summer of Code, 2022

As announced on the [Linux Foundation Wiki](https://wiki.linuxfoundation.org/gsoc/2022-gsoc-zephyr), this project was selected for the [Google Summer of Code](https://summerofcode.withgoogle.com/) under the umbrella of the [Linux Foundation](https://linuxfoundation.org).

Students! Your mission, should you choose to accept it, is:
1. Get this module in shape for upstreaming to the [Zephyr Project's GitHub](https://github.com/zephyrproject-rtos) (**MANDATORY**)
1. Ensure the module follows appropriate [coding guidelines](https://docs.zephyrproject.org/latest/contribute/coding_guidelines/index.html) and satisfies [module requirements](https://docs.zephyrproject.org/latest/guides/modules.html) (**MANDATORY**)
1. Perform additional integration of Thrift low-level transports such as TLS, and Memory Buffers (**One Or More**)
1. Perform additional integration of Thrift transports such as zlib, and HTTP (**One Or More**)
1. Perform additional integration of Thrift protocols such as Compact, and JSON (**One Or More**)
1. Author additional sample applications using [supported boards](https://docs.zephyrproject.org/latest/boards/index.html) or [Qemu](https://docs.zephyrproject.org/latest/guides/networking/qemu_setup.html) (**MANDATORY**)
1. Author additional tests and generate coverage reports using the [Zephyr Test Framework](https://docs.zephyrproject.org/latest/guides/test/ztest.html) (**MANDATORY**)

Students have access to 2 highly-skilled mentors, as well as the (fantastic!) [Zephyr Developer Community](https://www.zephyrproject.org/community/) via [Discord](https://discord.com/invite/zephyrproject). Naturally, there is also a vast quantity of [Zephyr documentation](https://docs.zephyrproject.org/latest/) and [Thrift documentation](https://thrift.apache.org/docs). Additionally, to facilitate wicked-cool demo videos, the successful applicant may receive some swag in the form of dev boards, stickers, and other stuff.

Truthfully, this is a challenging project. Aside from the vast quantity of highly-sought-after skills that the successful padawan would learn, they would also earn a respectable amount of geek-cred. Did we mention stickers?

For those interested, please **<big>[REGISTER HERE](https://summerofcode.withgoogle.com/register/contributor)<big>**. In the application, please consicely describe
* relevant experience with the Zephyr RTOS
* relevant experience with Apache Thrift
* a list of milestones (deliverables + dates) taken from the above mission statement
* a paragraph describing rationale for the proposed deliverables and how they would benefit the Zephyr + Linux developer community
* a YouTube video successfully demonstrating reproduced build + run of each of the tests and sample applications in this project

May the odds be ever in your favour!

# Project Details

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
