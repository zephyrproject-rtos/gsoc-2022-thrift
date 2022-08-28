# Thrift for Zephyr

## Build Status

[![Build](https://github.com/zephyrproject-rtos/gsoc-2022-thrift/actions/workflows/build.yml/badge.svg)](https://github.com/zephyrproject-rtos/gsoc-2022-thrift/actions)

## What is Thrift?

[Thrift](https://github.com/apache/thrift) is an [IDL](https://en.wikipedia.org/wiki/Interface_description_language) specification, [RPC](https://en.wikipedia.org/wiki/Remote_procedure_call) framework, and [code generator](https://en.wikipedia.org/wiki/Automatic_programming). It works across all major operating systems, supports over 27 programming languages, 7 protocols, and 6 low-level transports. Thrift was originally developed at [Facebook in 2007](https://thrift.apache.org/static/files/thrift-20070401.pdf). Subsequently, it was donated to the [Apache Software Foundation](https://www.apache.org/). Thrift supports a rich set of types and data structures, and abstracts away transport and protocol details, which lets developers focus on application logic.

As its name suggests, Thrift is designed to be lightweight and efficient. While saving IoT developers from writing lots of boilerplate, the code size can remain tight to fit in embedded platforms. Moreover, Thrift's cross-language capability allows the client and the server be written in different languages. Developers are not limited to use C or C++, and can drive the application with their favourite technology.

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
west init -m https://github.com/zephyrproject-rtos/gsoc-2022-thrift --mr main ${WS}
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

## Supported Stacks
- Low-Level Transports: Domain, Socket, TLS
- Transport Wrappers: Buffer, Zlib
- Protocols: Binary, Compact
- Servers: Simple

### Incompatibilities with Standard APIs
MbedTLS is used in this module to replace the original OpenSSL TLS implementation, which leads to some inconsistency with the standard Thrift APIs:
- SSLv3 is not supported
- `TSSLSocketFactory::ciphers()` takes no effect: all ciphersuites available in the system is allowed
- Loading certificates and keys from files is not supported

## Sample and Tests
[hello.thrift](thrift/hello.thrift) in this repository is a good start for getting familiar with the Thrift IDL:
```
service Hello {
    void ping();
    string echo(1: string msg);
    i32 counter();
}
```
It describes a service `Hello` with 3 methods. The first one takes no argument and returns no value. The second one has a string-type argument and returns a string. The third one takes no argument and returns a 32-bit integer.

The sample application includes a client a server which implemented the service above. The client-side code is under [samples/lib/thrift/hello_client](samples/lib/thrift/hello_client) and the server is under [samples/lib/thrift/hello_server](samples/lib/thrift/hello_server). See the [documentation](samples/lib/thrift/README.rst) of the sample for more information.

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

## Contributors
[Brandon Thomas Ruggles](https://github.com/brandontruggles)

[Christopher Friedt](https://github.com/cfriedt)

[Stephanos Ioannidis](https://github.com/stephanosio)

[Young Mei](https://github.com/SdtElectronics)

## License
Apache-2.0
