/*
 * Copyright 2022 Young Mei
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace apache::thrift::transport {

enum ThriftTLScertificateType {
  Thrift_TLS_CA_CERT_TAG,
  Thrift_TLS_SERVER_CERT_TAG,
  Thrift_TLS_PRIVATE_KEY,
};

}
