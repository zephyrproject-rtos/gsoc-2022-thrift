# Copyright 2022 Meta
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

set(THRIFT_UPSTREAM "${ZEPHYR_THRIFT_MODULE_DIR}/.apache-thrift")

zephyr_library()
zephyr_include_directories(include)
zephyr_include_directories(src)
zephyr_include_directories(${THRIFT_UPSTREAM}/lib/cpp/src)

zephyr_library_sources(
  src/_stat.c
  src/thrift/concurrency/Mutex.cpp
  src/thrift/server/TFDServer.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/protocol/TProtocol.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/server/TConnectedClient.cpp
  src/thrift/server/TServerFramework.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/server/TSimpleServer.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/transport/SocketCommon.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/transport/TBufferTransports.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/transport/TFDTransport.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/transport/TTransportException.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/transport/TServerSocket.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/transport/TSocket.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/TApplicationException.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/TOutput.cpp
)

zephyr_library_sources_ifdef(CONFIG_THRIFT_SSL_SOCKET
  src/thrift/transport/TSSLSocket.cpp
  src/thrift/transport/TSSLServerSocket.cpp
)

zephyr_library_sources_ifdef(CONFIG_THRIFT_ZLIB_TRANSPORT
  muzic/defl_static.c
  muzic/genlz77.c
  muzic/inflater.c
  muzic/zlib_comp.c
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/transport/TZlibTransport.cpp
)

# needed because std::iterator was deprecated with -std=c++17
zephyr_library_compile_options(-Wno-deprecated-declarations)
