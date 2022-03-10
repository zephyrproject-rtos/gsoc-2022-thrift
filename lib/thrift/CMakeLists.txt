# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

set(THRIFT_UPSTREAM "${ZEPHYR_CURRENT_MODULE_DIR}/.upstream")

include(${ZEPHYR_CURRENT_MODULE_DIR}/cmake/thrift.cmake)

# needed in order to expose some C++ class definitions in Newlib
target_compile_definitions(app PUBLIC _GLIBCXX_HAS_GTHREADS=1)

zephyr_library()
zephyr_include_directories(include)
zephyr_include_directories(${THRIFT_UPSTREAM}/lib/cpp/src)

zephyr_library_sources(
  src/thrift/server/TFDServer.cpp
  src/thrift/server/TSimpleServer.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/protocol/TProtocol.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/server/TConnectedClient.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/server/TServerFramework.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/transport/TFDTransport.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/transport/TTransportException.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/transport/TBufferTransports.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/TApplicationException.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/TOutput.cpp
  ${THRIFT_UPSTREAM}/lib/cpp/src/thrift/TOutput.cpp
)
