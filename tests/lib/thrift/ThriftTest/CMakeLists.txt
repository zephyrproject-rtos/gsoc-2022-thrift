# Copyright 2022 Meta
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(thrift_test)

set(THRIFT_UPSTREAM "${ZEPHYR_THRIFT_MODULE_DIR}/.apache-thrift")

include(${ZEPHYR_THRIFT_MODULE_DIR}/cmake/thrift.cmake)

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
    # v0.16: ubuntu packaged thrift compiler does not support 'uuid' type
    "${THRIFT_UPSTREAM}/test/v0.16/ThriftTest.thrift"
    ""
    ${generated_sources}
)

target_sources(app PRIVATE ${app_sources})

# needed because std::iterator was deprecated with -std=c++17
target_compile_options(app PRIVATE -Wno-deprecated-declarations)
