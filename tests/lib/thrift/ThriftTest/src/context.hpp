/*
 * Copyright 2022 Young Mei
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>

#include <pthread.h>

#include <thrift/server/TSimpleServer.h>

#include "ThriftTest.h"

using namespace apache::thrift::server;
using namespace thrift::test;

struct ctx {
  enum {
    SERVER,
    CLIENT,
  };

  std::array<int, CLIENT + 1> fds;
  std::unique_ptr<ThriftTestClient> client;
  std::unique_ptr<TServer> server;
  pthread_t server_thread;
};

extern ctx context;
