/*
 * Copyright 2022 Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef __ZEPHYR__
#include <zephyr.h>
#endif

#include <cstdlib>
#include <iostream>

#include <fcntl.h>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TFDTransport.h>
#include <thrift/transport/TSocket.h>

#include "Hello.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

#ifdef __ZEPHYR__
int main(void)
#else
int main(int argc, char** argv)
#endif
{
  std::string my_addr;

#ifdef __ZEPHYR__
  my_addr = CONFIG_NET_CONFIG_PEER_IPV4_ADDR;
#else
  if (argc >= 2) {
    my_addr = std::string(argv[1]);
  } else {
    my_addr = "192.0.2.1";
  }
#endif

  int port = 4242;
  std::shared_ptr<TTransport> trans(new TSocket(my_addr, port));
  std::shared_ptr<TTransport> transport(new TBufferedTransport(trans));
  std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  HelloClient client(protocol);

  try {
    transport->open();
    client.ping();
    std::string s;
    client.echo(s, "Hello, world!");
    for (int i = 0; i < 5; ++i) {
      client.counter();
    }
    transport->close();
  } catch (std::exception& e) {
    printf("caught exception: %s\n", e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
