/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>

#include <array>
#include <memory>
#include <stdexcept>

#ifdef CONFIG_SOC_POSIX
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#else
#include <posix/pthread.h>
#include <posix/unistd.h>
#include <posix/sys/socket.h>
#endif

#include "Hello.h"
#include "HelloHandler.h"
#include "thrift/transport/TFDServer.h"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TFDTransport.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace std;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

struct ctx
{
    enum
    {
        SERVER,
        CLIENT,
    };

    std::array<int, CLIENT + 1> fds;
    pthread_t server_thread;
    shared_ptr<TServer> server;
};
static ctx context;

static void *server_func(void *arg)
{
    (void)arg;

    printk("Starting the server...\n");
    context.server->serve();
    printk("Server is done\n");

    return nullptr;
}

static void setup(void)
{
    int rv;

    // create the communication channel
    rv = socketpair(AF_UNIX, SOCK_STREAM, 0, &context.fds.front());
    assert(rv == 0);
    // zassert_equal(0, rv, "socketpair failed: %d", rv);

    // set up server
    shared_ptr<HelloHandler> handler(new HelloHandler());
    shared_ptr<TProcessor> processor(new HelloProcessor(handler));
    shared_ptr<TServerTransport> strans(new TFDServer(context.fds[ctx::SERVER]));
    context.server = shared_ptr<TServer>(new TSimpleServer(processor,
                                                           strans,
                                                           std::make_shared<TBufferedTransportFactory>(),
                                                           std::make_shared<TBinaryProtocolFactory>()));

    // start the server
    rv = pthread_create(&context.server_thread, nullptr, server_func, nullptr);
    assert(rv == 0);
    // zassert_equal(0, rv, "pthread_create failed: %d", rv);
}

static void teardown(void)
{
    void *unused;

    // Note: ATM, server->stop() does nothing for TFDServer
    // since we have not implemented interrupt() and interruptChildren()
    //
    // stop() {
    //   serverTransport_->interruptChildren();
    //   serverTransport_->interrupt();
    // }
    context.server->stop();

#ifdef CONFIG_SOC_POSIX
    // should interrupt read(2), but unfortunately does not allow us to join cleanly
    kill(getpid(), SIGINT);
#endif

    pthread_join(context.server_thread, &unused);

    for (auto &fd : context.fds)
    {
        close(fd);
    }
}

static void test_hello(void)
{
    std::shared_ptr<TTransport> trans(new TFDTransport(context.fds[ctx::CLIENT]));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(trans));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    HelloClient client(protocol);

    transport->open();
    client.ping();
    string s;
    client.echo(s, "Hello, world!");
    for (int i = 0; i < 5; ++i)
    {
        client.counter();
    }
}

void test_main(void)
{

    ztest_test_suite(thrift_hello,
                     ztest_unit_test_setup_teardown(test_hello, setup, teardown));
    ztest_run_test_suite(thrift_hello);
}
