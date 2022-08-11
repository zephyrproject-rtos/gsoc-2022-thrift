/*
 * Copyright 2022 Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef CONFIG_ARCH_POSIX

#include_next <sys/poll.h>

#else

/*
 * Thrift mistakenly includes <sys/poll.h> instead of <poll.h>
 * See https://pubs.opengroup.org/onlinepubs/9699919799/functions/poll.html
 */

#include <zephyr/posix/poll.h>

/* sneaking this in here to avoid changing zephyr's <sys/ioctl.h> */
#ifndef FIONREAD
#define FIONREAD 42
#endif

/* sneaking this in here to avoid changing zephyr's <netdb.h> */
#define NI_MAXSERV 3

/* sneaking this in here to avoid changing zephyr's <sys/socket.h> */
struct linger {
  int l_onoff;  /* option on/off */
  int l_linger; /* linger time */
};

#endif
