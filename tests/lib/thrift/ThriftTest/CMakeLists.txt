# Copyright 2022 Meta
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(thrift_test)

set(THRIFT_UPSTREAM "${CMAKE_CURRENT_SOURCE_DIR}/../../../../.upstream")

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/thrift.cmake)

FILE(GLOB app_sources 
    src/*.cpp
)

set(generated_sources "")
set(gen_dir ${ZEPHYR_BINARY_DIR}/misc/generated/thrift_ThriftTest)
list(APPEND generated_sources ${gen_dir}/gen-cpp/SecondService.cpp)
list(APPEND generated_sources ${gen_dir}/gen-cpp/SecondService.h)
list(APPEND generated_sources ${gen_dir}/gen-cpp/ThriftTest_constants.cpp)
list(APPEND generated_sources ${gen_dir}/gen-cpp/ThriftTest_constants.h)
list(APPEND generated_sources ${gen_dir}/gen-cpp/ThriftTest.cpp)
list(APPEND generated_sources ${gen_dir}/gen-cpp/ThriftTest.h)
list(APPEND generated_sources ${gen_dir}/gen-cpp/ThriftTest_types.cpp)
list(APPEND generated_sources ${gen_dir}/gen-cpp/ThriftTest_types.h)
list(APPEND app_sources ${generated_sources})

thrift(
    app
    cpp
    :no_skeleton
    ${gen_dir}
    # FIXME: is there a variable that can be used here to refer to the module root?
    "${THRIFT_UPSTREAM}/test/ThriftTest.thrift"
    ""
    ${generated_sources}
)

target_sources(app PRIVATE ${app_sources})
