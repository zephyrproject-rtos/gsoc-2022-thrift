#include <thrift/transport/TFDTransport.h>

#include "TFDServer.h"

namespace apache
{
    namespace thrift
    {
        namespace transport
        {

            bool TFDServer::isOpen() const
            {
                return true;
            }

            std::shared_ptr<TTransport> TFDServer::acceptImpl()
            {
                return xport;
            }

            THRIFT_SOCKET TFDServer::getSocketFD()
            {
                return fd;
            }

            void TFDServer::close()
            {
                ::close(fd);
                fd = -1;
            }

            TFDServer::TFDServer(int fd) : fd(fd), xport(std::shared_ptr<TTransport>(new TFDTransport(fd))) {}
            TFDServer::~TFDServer()
            {
                close();
            }
        }
    }
}