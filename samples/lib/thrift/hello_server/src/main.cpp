/*
 * Copyright 2022 Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef __ZEPHYR__
#include <zephyr/zephyr.h>
#endif

#include <cstdio>
#include <cstdlib>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TSSLServerSocket.h>
#include <thrift/transport/TSSLSocket.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TZlibTransport.h>

#include "Hello.h"
#include "HelloHandler.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

#ifndef IS_ENABLED
#define IS_ENABLED(flag) flag
#endif

#ifndef CONFIG_THRIFT_COMPACT_PROTOCOL
#define CONFIG_THRIFT_COMPACT_PROTOCOL 0
#endif

#ifndef CONFIG_THRIFT_ZLIB_TRANSPORT
#define CONFIG_THRIFT_ZLIB_TRANSPORT 0
#endif

#ifndef CONFIG_THRIFT_SSL_SOCKET
#define CONFIG_THRIFT_SSL_SOCKET 0
#endif

#ifdef __ZEPHYR__
int main(void)
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
  std::shared_ptr<TServerTransport> serverTransport;
  if (IS_ENABLED(CONFIG_THRIFT_SSL_SOCKET)) {
    const int port = 4242;
    std::shared_ptr<TSSLSocketFactory> socketFactory(new TSSLSocketFactory());
    socketFactory->server(true);
#ifdef __ZEPHYR__
    socketFactory->loadCertificateFromBuffer(
#include "../../hello_common/qemu-cert.pem"
    );
    socketFactory->loadPrivateKeyFromBuffer(
#include "../../hello_common/qemu-key.pem"
    );
    socketFactory->loadTrustedCertificatesFromBuffer(
#include "../../hello_common/native-cert.pem"
    );
#else
    socketFactory->loadCertificateFromBuffer(
#include "../../hello_common/native-cert.pem"
    );
    socketFactory->loadPrivateKeyFromBuffer(
#include "../../hello_common/native-key.pem"
    );
    socketFactory->loadTrustedCertificatesFromBuffer(
#include "../../hello_common/qemu-cert.pem"
    );
#endif
    serverTransport = std::make_shared<TSSLServerSocket>("0.0.0.0", port, socketFactory);
  } else {
    serverTransport = std::make_shared<TServerSocket>(my_addr, port);
  }
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

  try {
    server.serve();
  } catch (std::exception& e) {
    printf("caught exception: %s\n", e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
