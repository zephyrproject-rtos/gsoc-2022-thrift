# Copyright 2022 Meta
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

set(THRIFT_UPSTREAM "${CMAKE_CURRENT_SOURCE_DIR}/../../.upstream")

# needed in order to expose some C++ class definitions in Newlib
target_compile_definitions(app PUBLIC _GLIBCXX_HAS_GTHREADS=1)

zephyr_library()
zephyr_include_directories(include)
zephyr_include_directories(muzic/include)
zephyr_include_directories(${THRIFT_UPSTREAM}/lib/cpp/src)

zephyr_library_sources(
  src/_stat.c
  src/thrift/concurrency/Mutex.cpp
  src/thrift/server/TFDServer.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/protocol/TProtocol.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/server/TConnectedClient.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/server/TServerFramework.cpp
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

zephyr_library_sources_ifdef(CONFIG_THRIFT_ZLIB
  muzic/src/defl_static.c
  muzic/src/genlz77.c
  muzic/src/inflater.c
  muzic/src/zlib_comp.c
)

zephyr_library_sources_ifdef(CONFIG_THRIFT_ZLIB_TRANSPORT
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/transport/TZlibTransport.cpp
)
