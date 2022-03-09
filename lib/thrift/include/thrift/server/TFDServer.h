#pragma once

#include <memory>

#include <thrift/transport/TServerTransport.h>

namespace apache
{
    namespace thrift
    {
        namespace transport
        {

            class TFDServer : public TServerTransport
            {
            public:
                TFDServer(int fd);
                virtual ~TFDServer();

                virtual bool isOpen() const;
                virtual THRIFT_SOCKET getSocketFD();
                virtual void close();

            protected:
                TFDServer() : TFDServer(-1){};
                virtual std::shared_ptr<TTransport> acceptImpl();

                int fd;
                std::shared_ptr<TTransport> xport;
            };
        }
    }
} // apache::thrift::transport
