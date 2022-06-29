/*
 * Copyright 2022 Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef __ZEPHYR__
#include <zephyr.h>
#endif

#include <cstdio>
#include <cstdlib>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include "Hello.h"
#include "HelloHandler.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

#ifdef __ZEPHYR__
void main(void)
#else
int main(int argc, char** argv)
#endif
{
  std::string my_addr;

#ifdef __ZEPHYR__
  my_addr = CONFIG_NET_CONFIG_MY_IPV4_ADDR;
#else
  if (argc < 2) {
    printf("usage: %s <ip>\n", argv[0]);
    return EXIT_FAILURE;
  }

  my_addr = std::string(argv[1]);
#endif

  int port = 4242;
  std::shared_ptr<HelloHandler> handler(new HelloHandler());
  std::shared_ptr<TProcessor> processor(new HelloProcessor(handler));
  std::shared_ptr<TServerTransport> serverTransport(new TServerSocket(my_addr, port));
  std::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  std::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);

  try {
    server.serve();
  } catch (std::exception& e) {
    printf("caught exception: %s\n", e.what());
#ifndef __ZEPHYR__
    return EXIT_FAILURE;
#endif
  }

#ifndef __ZEPHYR__
  return EXIT_SUCCESS;
#endif
}
