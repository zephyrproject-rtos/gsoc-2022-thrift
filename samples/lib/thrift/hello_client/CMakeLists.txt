# Copyright 2022 Meta
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(thrift_hello_server)

FILE(GLOB app_sources
    src/*.cpp
)

include(${ZEPHYR_THRIFT_MODULE_DIR}/cmake/thrift.cmake)

set(generated_sources "")
set(gen_dir ${ZEPHYR_BINARY_DIR}/misc/generated/thrift_hello)
list(APPEND generated_sources ${gen_dir}/gen-cpp/hello_types.h)
list(APPEND generated_sources ${gen_dir}/gen-cpp/Hello.cpp)
list(APPEND generated_sources ${gen_dir}/gen-cpp/Hello.h)
list(APPEND app_sources ${generated_sources})

thrift(
    app
    cpp
    :no_skeleton
    ${gen_dir}
    "${ZEPHYR_THRIFT_MODULE_DIR}/thrift/hello.thrift"
    ""
    ${generated_sources}
)

target_sources(app PRIVATE ${app_sources})

# needed because std::iterator was deprecated with -std=c++17
target_compile_options(app PRIVATE -Wno-deprecated-declarations)
