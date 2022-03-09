# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(thrift-for-zephyr LANGUAGES CXX VERSION 1.0.0)

target_include_directories(thrift-for-zephyr PRIVATE ${CMAKE_BINARY_DIR}/lib/include include)

target_sources(thrift-for-zephyr PRIVATE
  src/thrift/server/TFDServer.cpp
)
