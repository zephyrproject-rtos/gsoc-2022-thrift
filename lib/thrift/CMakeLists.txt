# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

include(${ZEPHYR_CURRENT_MODULE_DIR}/cmake/thrift.cmake)

# needed in order to expose some C++ class definitions in Newlib
target_compile_definitions(app PUBLIC _GLIBCXX_HAS_GTHREADS=1)

zephyr_library()
zephyr_include_directories(include)

zephyr_library_sources(
  src/thrift/server/TFDServer.cpp
)
