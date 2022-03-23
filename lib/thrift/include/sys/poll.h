#pragma once

/*
 * Thrift mistakenly includes <sys/poll.h> instead of <poll.h>
 * See https://pubs.opengroup.org/onlinepubs/9699919799/functions/poll.html
 */

#include <poll.h>

/* sneaking this in here to avoid changing zephyr's <sys/ioctl.h> */
#ifndef FIONREAD
#define FIONREAD 42
#endif

/* sneaking this in here to avoid changing zephyr's <netdb.h> */
#define NI_MAXSERV 3

/* sneaking this in here to avoid changing zephyr's <sys/socket.h> */
struct linger
{
    int l_onoff;  /* option on/off */
    int l_linger; /* linger time */
};
