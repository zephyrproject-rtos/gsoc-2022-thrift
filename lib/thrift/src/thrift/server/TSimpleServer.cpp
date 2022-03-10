#include <thrift/server/TSimpleServer.h>

using namespace std;
using namespace apache::thrift;

namespace apache
{
    namespace thrift
    {
        namespace server
        {

            TSimpleServer::TSimpleServer(
                const shared_ptr<TProcessorFactory> &processorFactory,
                const shared_ptr<TServerTransport> &serverTransport,
                const shared_ptr<TTransportFactory> &transportFactory,
                const shared_ptr<TProtocolFactory> &protocolFactory)
                : TServerFramework(processorFactory, serverTransport, transportFactory, protocolFactory)
            {
            }

            TSimpleServer::TSimpleServer(
                const shared_ptr<TProcessor> &processor,
                const shared_ptr<TServerTransport> &serverTransport,
                const shared_ptr<TTransportFactory> &transportFactory,
                const shared_ptr<TProtocolFactory> &protocolFactory)
                : TServerFramework(processor, serverTransport, transportFactory, protocolFactory)
            {
            }

            TSimpleServer::TSimpleServer(
                const shared_ptr<TProcessorFactory> &processorFactory,
                const shared_ptr<TServerTransport> &serverTransport,
                const shared_ptr<TTransportFactory> &inputTransportFactory,
                const shared_ptr<TTransportFactory> &outputTransportFactory,
                const shared_ptr<TProtocolFactory> &inputProtocolFactory,
                const shared_ptr<TProtocolFactory> &outputProtocolFactory)
                : TServerFramework(processorFactory, serverTransport, inputTransportFactory, outputTransportFactory, inputProtocolFactory, outputProtocolFactory)
            {
            }

            TSimpleServer::TSimpleServer(
                const shared_ptr<TProcessor> &processor,
                const shared_ptr<TServerTransport> &serverTransport,
                const shared_ptr<TTransportFactory> &inputTransportFactory,
                const shared_ptr<TTransportFactory> &outputTransportFactory,
                const shared_ptr<TProtocolFactory> &inputProtocolFactory,
                const shared_ptr<TProtocolFactory> &outputProtocolFactory)
                : TServerFramework(processor, serverTransport, inputTransportFactory, outputTransportFactory, inputProtocolFactory, outputProtocolFactory)
            {
            }

            TSimpleServer::~TSimpleServer()
            {
            }

            void TSimpleServer::serve()
            {
            }

            void TSimpleServer::stop()
            {
            }

            int64_t TSimpleServer::getConcurrentClientLimit() const
            {
                return 1;
            }

            int64_t TSimpleServer::getConcurrentClientCount() const
            {
                return 1;
            }

            int64_t TSimpleServer::getConcurrentClientCountHWM() const
            {
                return 1;
            }

            void TSimpleServer::setConcurrentClientLimit(int64_t newLimit)
            {
            }

            void TSimpleServer::onClientConnected(const shared_ptr<TConnectedClient> &pClient)
            {
            }

            void TSimpleServer::onClientDisconnected(TConnectedClient *pClient)
            {
            }
        }
    }
} // server
