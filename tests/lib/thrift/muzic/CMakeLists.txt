# Copyright 2022 Young Mei
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(muzic_test)

set(MUZIC_BASE "${ZEPHYR_THRIFT_MODULE_DIR}/lib/thrift/")

zephyr_include_directories(${MUZIC_BASE}/include)

target_sources(app PRIVATE 
  src/integration.c
  ${MUZIC_BASE}/muzic/defl_static.c
  ${MUZIC_BASE}/muzic/genlz77.c
  ${MUZIC_BASE}/muzic/inflater.c
  ${MUZIC_BASE}/muzic/zlib_comp.c
)

