// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <memory>
#include <vector>

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

                virtual void interrupt() override;
                virtual void interruptChildren() override;

            protected:
                TFDServer() : TFDServer(-1){};
                virtual std::shared_ptr<TTransport> acceptImpl();

                int fd;
                std::vector<std::shared_ptr<TTransport>> children;
            };
        }
    }
} // apache::thrift::transport
