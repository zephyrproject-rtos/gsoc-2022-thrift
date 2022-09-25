# Thrift for Zephyr

## Build Status

[![Build](https://github.com/zephyrproject-rtos/gsoc-2022-thrift/actions/workflows/build.yml/badge.svg)](https://github.com/zephyrproject-rtos/gsoc-2022-thrift/actions)
[![Build (POSIX)](https://github.com/zephyrproject-rtos/gsoc-2022-thrift/actions/workflows/build-posix.yml/badge.svg)](https://github.com/zephyrproject-rtos/gsoc-2022-thrift/actions/workflows/build-posix.yml)

## What is Thrift?

[Thrift](https://github.com/apache/thrift) is an [IDL](https://en.wikipedia.org/wiki/Interface_description_language) specification, [RPC](https://en.wikipedia.org/wiki/Remote_procedure_call) framework, and [code generator](https://en.wikipedia.org/wiki/Automatic_programming). It works across all major operating systems, supports over 27 programming languages, 7 protocols, and 6 low-level transports. Thrift was originally developed at [Facebook in 2007](https://thrift.apache.org/static/files/thrift-20070401.pdf). Subsequently, it was donated to the [Apache Software Foundation](https://www.apache.org/). Thrift supports a rich set of types and data structures, and abstracts away transport and protocol details, which lets developers focus on application logic.

As its name suggests, Thrift is designed to be lightweight and efficient. While saving IoT developers from writing lots of boilerplate, the code size can remain tight to fit in embedded platforms. Moreover, Thrift's cross-language capability allows the client and the server be written in different languages. Developers are not limited to use C or C++, and can drive the application with their favourite technology.

## Getting Started

Before getting started, make sure you have a proper Zephyr development
environment by following the official
[Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html).

Then, copy [99-thrift.yaml](submanifests/99-thrift.yaml)
in this repository to `zephyrproject/zephyr/submanifests/`, and run `west update` again:
```shell
cat << EOF > submanifests/99-thrift.yaml
manifest:
  defaults:
    remote: upstream

  remotes:
    - name: upstream
      url-base: https://github.com/zephyrproject-rtos
    - name: cfriedt
      url-base: https://github.com/cfriedt

  projects:
    - name: gsoc-2022-thrift
      path: modules/lib/thrift
      revision: main
    - name: thrift
      remote: cfriedt
      path: modules/lib/thrift/.upstream
EOF
west update
```

### Installing Thrift Binary

**MacOS**
```shell
brew install thrift
```

**Ubuntu**
```shell
apt install -y libboost-all-dev thrift-compiler libthrift-dev
```

## Supported Features
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
