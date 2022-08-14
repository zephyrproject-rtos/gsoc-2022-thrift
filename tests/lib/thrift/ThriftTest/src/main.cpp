/*
 * Copyright 2022 Young Mei
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

#include <zephyr/ztest.h>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TFDTransport.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TZlibTransport.h>

#include "context.hpp"
#include "server.hpp"
#include "thrift/server/TFDServer.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

ctx context;

static K_THREAD_STACK_DEFINE(ThriftTest_server_stack, CONFIG_THRIFTTEST_SERVER_STACK_SIZE);

static void* server_func(void* arg) {
  (void)arg;

  context.server->serve();

  return nullptr;
}

static std::unique_ptr<ThriftTestClient> setup_client() {
  std::shared_ptr<TTransport> trans(new TFDTransport(context.fds[ctx::CLIENT]));
  std::shared_ptr<TTransport> transport(new TBufferedTransport(trans));
  if (IS_ENABLED(CONFIG_THRIFT_ZLIB_TRANSPORT)) {
    transport = std::make_shared<TZlibTransport>(trans);
  } else {
    transport = std::make_shared<TBufferedTransport>(trans);
  }
  std::shared_ptr<TProtocol> protocol;
  if (IS_ENABLED(CONFIG_THRIFT_COMPACT_PROTOCOL)) {
    protocol = std::make_shared<TCompactProtocol>(transport);
  } else {
    protocol = std::make_shared<TBinaryProtocol>(transport);
  }
  transport->open();
  return std::unique_ptr<ThriftTestClient>(new ThriftTestClient(protocol));
}

static std::unique_ptr<TServer> setup_server() {
  std::shared_ptr<TestHandler> handler(new TestHandler());
  std::shared_ptr<TProcessor> processor(new ThriftTestProcessor(handler));
  std::shared_ptr<TServerTransport> serverTransport(new TFDServer(context.fds[ctx::SERVER]));
  std::shared_ptr<TTransportFactory> transportFactory;
  if (IS_ENABLED(CONFIG_THRIFT_ZLIB_TRANSPORT)) {
    transportFactory = std::make_shared<TZlibTransportFactory>();
  } else {
    transportFactory = std::make_shared<TBufferedTransportFactory>();
  }
  std::shared_ptr<TProtocolFactory> protocolFactory;
  if (IS_ENABLED(CONFIG_THRIFT_COMPACT_PROTOCOL)) {
    protocolFactory = std::make_shared<TCompactProtocolFactory>();
  } else {
    protocolFactory = std::make_shared<TBinaryProtocolFactory>();
  }
  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  return std::unique_ptr<TServer>(
      new TSimpleServer(processor, serverTransport, transportFactory, protocolFactory));
}

static void thrift_test_before(void* data) {
  ARG_UNUSED(data);
  int rv;

  pthread_attr_t attr;
  pthread_attr_t* attrp = &attr;

  if (IS_ENABLED(CONFIG_ARCH_POSIX)) {
    attrp = NULL;
  } else {
    rv = pthread_attr_init(attrp);
    zassert_equal(0, rv, "pthread_attr_init failed: %d", rv);
    rv = pthread_attr_setstack(attrp, ThriftTest_server_stack, CONFIG_THRIFTTEST_SERVER_STACK_SIZE);
    zassert_equal(0, rv, "pthread_attr_setstack failed: %d", rv);
  }

  // create the communication channel
  rv = socketpair(AF_UNIX, SOCK_STREAM, 0, &context.fds.front());
  zassert_equal(0, rv, "socketpair failed: %d\n", rv);

  // set up server
  context.server = setup_server();

  // set up client
  context.client = setup_client();

  // start the server
  rv = pthread_create(&context.server_thread, attrp, server_func, nullptr);
  zassert_equal(0, rv, "pthread_create failed: %d", rv);
}

static void thrift_test_after(void* data) {
  ARG_UNUSED(data);
  void* unused;

  context.server->stop();

  pthread_join(context.server_thread, &unused);

  context.server.reset();
  context.client.reset();

  for (auto& fd : context.fds) {
    close(fd);
    fd = -1;
  }
}

ZTEST_SUITE(thrift, NULL, NULL, thrift_test_before, thrift_test_after, NULL);
