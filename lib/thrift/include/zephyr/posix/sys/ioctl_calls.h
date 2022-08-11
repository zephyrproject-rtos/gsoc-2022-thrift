/*
 * Copyright (c) 2022 Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

__syscall int zephyr_ioctl(int fd, unsigned long request, va_list args);

#ifdef __cplusplus
}
#endif

#include <syscalls/ioctl_calls.h>
