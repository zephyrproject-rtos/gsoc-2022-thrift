/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>

#include "Hello.h"
#include "HelloHandler.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

int main(int argc, char **argv)
{

    int port = 4242;
    ::std::shared_ptr<HelloHandler> handler(new HelloHandler());
    ::std::shared_ptr<TProcessor> processor(new HelloProcessor(handler));
    ::std::shared_ptr<TServerTransport> serverTransport(new TServerSocket(CONFIG_NET_CONFIG_MY_IPV4_ADDR, port));
    ::std::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    ::std::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
    TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);

    try {
        printk("calling server.serve()\n");
        server.serve();
        printk("returned from server.serve()\n");
    } catch(std::exception& e) {
        printk("caught exception: %s\n", e.what());
    }

    return 0;
}
