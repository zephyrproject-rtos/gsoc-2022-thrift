/*
 * Copyright 2022 Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <array>
#include <system_error>

#include <errno.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <thrift/transport/TFDTransport.h>

#include <logging/log.h>
#include <zephyr.h>

#include "thrift/server/TFDServer.h"

LOG_MODULE_REGISTER(TFDServer, LOG_LEVEL_DBG);

using namespace std;

namespace apache {
namespace thrift {
namespace transport {

class xport : public TVirtualTransport<xport> {
public:
  xport() : xport(-1, -1) {}
  xport(int fd) : xport(fd, eventfd(0, EFD_SEMAPHORE)) {}
  xport(int fd, int efd) : fd(fd), efd(efd) {}
  ~xport() { ::close(efd); }

  virtual uint32_t read_virt(uint8_t* buf, uint32_t len) override {
    int r;
    array<pollfd, 2> pollfds = {
        (pollfd){
            .fd = fd,
            .events = POLLIN,
            .revents = 0,
        },
        (pollfd){
            .fd = efd,
            .events = POLLIN,
            .revents = 0,
        },
    };

    r = poll(&pollfds.front(), pollfds.size(), -1);
    if (r == -1) {
      LOG_ERR("failed to poll fds %d, %d: %d", fd, efd, errno);
      throw system_error(errno, system_category(), "poll");
    }

    if (pollfds[0].revents & POLLIN) {
      r = ::read(fd, buf, len);
      if (r == -1) {
        LOG_ERR("failed to read %d bytes from fd %d: %d", len, fd, errno);
        system_error(errno, system_category(), "read");
      }

      __ASSERT_NO_MSG(r > 0);

      return uint32_t(r);
    }

    __ASSERT_NO_MSG(pollfds[1].revents & POLLIN);

    return 0;
  }

  virtual void write_virt(const uint8_t* buf, uint32_t len) override {
    for (int r = 0; len > 0; buf += r, len -= r) {
      r = ::write(fd, buf, len);
      if (r == -1) {
        LOG_ERR("writing %u bytes to fd %d failed: %d", len, fd, errno);
        throw system_error(errno, system_category(), "write");
      }

      __ASSERT_NO_MSG(r > 0);
    }
  }

  void interrupt() {
    constexpr uint64_t x = 0xb7e;
    int r = ::write(efd, &x, sizeof(x));
    if (r == -1) {
      LOG_ERR("writing %zu bytes to fd %d failed: %d", sizeof(x), efd, errno);
      throw system_error(errno, system_category(), "write");
    }

    __ASSERT_NO_MSG(r > 0);
  }

protected:
  int fd;
  int efd;
};

TFDServer::TFDServer(int fd) : fd(fd) {}
TFDServer::~TFDServer() {}

bool TFDServer::isOpen() const {
  return true;
}

shared_ptr<TTransport> TFDServer::acceptImpl() {
  if (fd < 0) {
    throw TTransportException(TTransportException::UNKNOWN, "invalid fd");
  }

  children.push_back(shared_ptr<TTransport>(new xport(fd)));

  return children.back();
}

THRIFT_SOCKET TFDServer::getSocketFD() {
  return fd;
}

void TFDServer::close() {
  fd = -1;
}

void TFDServer::interrupt() {
  close();
}

void TFDServer::interruptChildren() {
  for (auto c : children) {
    auto child = reinterpret_cast<xport*>(c.get());
    child->interrupt();
  }
}
} // namespace transport
} // namespace thrift
} // namespace apache
