/*
 * Copyright 2022 Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>

#include <array>
#include <memory>
#include <stdexcept>

#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Hello.h"
#include "HelloHandler.h"
#include "thrift/server/TFDServer.h"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TFDTransport.h>
#include <thrift/transport/TServerSocket.h>

using namespace std;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

#define HELLO_SERVER_STACK_SIZE 4096
static K_THREAD_STACK_DEFINE(hello_server_stack, HELLO_SERVER_STACK_SIZE);

struct ctx {
  enum {
    SERVER,
    CLIENT,
  };

  std::array<int, CLIENT + 1> fds;
  std::shared_ptr<HelloClient> client;
  std::shared_ptr<TServer> server;
  pthread_t server_thread;
};

static ctx context;

static void* server_func(void* arg) {
  (void)arg;

  try {
    context.server->serve();
  } catch (...) {
  }

  return nullptr;
}

static std::shared_ptr<HelloClient> setup_client() {
  std::shared_ptr<TTransport> trans(new TFDTransport(context.fds[ctx::CLIENT]));
  std::shared_ptr<TTransport> transport(new TBufferedTransport(trans));
  std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  transport->open();
  return std::make_shared<HelloClient>(protocol);
}

std::shared_ptr<TServer> setup_server() {
  std::shared_ptr<HelloHandler> handler(new HelloHandler());
  std::shared_ptr<TProcessor> processor(new HelloProcessor(handler));
  std::shared_ptr<TServerTransport> serverTransport(new TFDServer(context.fds[ctx::SERVER]));
  std::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  std::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  return std::shared_ptr<TServer>(
      new TSimpleServer(processor, serverTransport, transportFactory, protocolFactory));
}

static void setup(void) {
  int rv;
  pthread_attr_t attr;
  pthread_attr_t* attrp = &attr;

  if (IS_ENABLED(CONFIG_ARCH_POSIX)) {
    attrp = NULL;
  } else {
    rv = pthread_attr_init(attrp);
    zassert_equal(0, rv, "pthread_attr_init failed: %d", rv);
    rv = pthread_attr_setstack(attrp, hello_server_stack, HELLO_SERVER_STACK_SIZE);
    zassert_equal(0, rv, "pthread_attr_setstack failed: %d", rv);
  }

  // create the communication channel
  rv = socketpair(AF_UNIX, SOCK_STREAM, 0, &context.fds.front());
  zassert_equal(0, rv, "socketpair failed: %d", rv);

  // set up server
  context.server = setup_server();

  // set up client
  context.client = setup_client();

  // start the server
  rv = pthread_create(&context.server_thread, attrp, server_func, nullptr);
  zassert_equal(0, rv, "pthread_create failed: %d", rv);
}

static void teardown(void) {
  void* unused;

  try {
    context.server->stop();
  } catch (...) {
  }

  zassert_ok(pthread_join(context.server_thread, &unused), "");

  context.client = nullptr;
  context.server = nullptr;

  for (auto& fd : context.fds) {
    close(fd);
  }
}

static void test_ping(void) {
  context.client->ping();
}

static void test_hello(void) {
  string s;

  context.client->echo(s, "Hello, world!");
  zassert_equal(s, "Hello, world!", "");
}

static void test_counter(void) {
  for (int i = 1; i <= 5; ++i) {
    zassert_equal(i, context.client->counter(), "");
  }
}

void test_main(void) {

  ztest_test_suite(thrift_hello, ztest_unit_test_setup_teardown(test_ping, setup, teardown),
                   ztest_unit_test_setup_teardown(test_hello, setup, teardown),
                   ztest_unit_test_setup_teardown(test_counter, setup, teardown));
  ztest_run_test_suite(thrift_hello);
}
