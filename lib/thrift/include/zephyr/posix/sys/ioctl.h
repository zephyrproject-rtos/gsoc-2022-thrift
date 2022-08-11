/*
 * Copyright 2022 Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#define FIONBIO 0x5421

#ifdef __cplusplus
extern "C" {
#endif

/* Zephyr's header does not include this crucial declaration */
int ioctl(int fd, unsigned long request, ...);

#include <zephyr/posix/sys/ioctl_calls.h>

#ifdef __cplusplus
}
#endif
