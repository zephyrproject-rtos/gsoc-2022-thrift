/*
 * Copyright 2022 Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>

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
  pthread_t server_thread;
  shared_ptr<TServer> server;
};
static ctx context;

static void* server_func(void* arg) {
  (void)arg;

  context.server->serve();

  return nullptr;
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
  shared_ptr<HelloHandler> handler(new HelloHandler());
  shared_ptr<TProcessor> processor(new HelloProcessor(handler));
  shared_ptr<TServerTransport> strans(new TFDServer(context.fds[ctx::SERVER]));
  context.server = shared_ptr<TServer>(
      new TSimpleServer(processor, strans, std::make_shared<TBufferedTransportFactory>(),
                        std::make_shared<TBinaryProtocolFactory>()));

  // start the server
  rv = pthread_create(&context.server_thread, attrp, server_func, nullptr);
  zassert_equal(0, rv, "pthread_create failed: %d", rv);
}

static void teardown(void) {
  void* unused;

  context.server->stop();

  pthread_join(context.server_thread, &unused);

  for (auto& fd : context.fds) {
    close(fd);
  }
}

static void test_hello(void) {
  std::shared_ptr<TTransport> trans(new TFDTransport(context.fds[ctx::CLIENT]));
  std::shared_ptr<TTransport> transport(new TBufferedTransport(trans));
  std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  HelloClient client(protocol);

  transport->open();
  client.ping();
  string s;
  client.echo(s, "Hello, world!");
  for (int i = 0; i < 5; ++i) {
    client.counter();
  }
}

void test_main(void) {

  ztest_test_suite(thrift_hello, ztest_unit_test_setup_teardown(test_hello, setup, teardown));
  ztest_run_test_suite(thrift_hello);
}
