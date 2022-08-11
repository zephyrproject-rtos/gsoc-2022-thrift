/*
 * Copyright 2022 Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef __ZEPHYR__
#include <zephyr/zephyr.h>
#else
#define printk printf
#endif

#include <iostream>

#include "Hello.h"

class HelloHandler : virtual public HelloIf {
public:
  HelloHandler() : count(0) {}

  void ping() { printk("ping\n"); }

  void echo(std::string& _return, const std::string& msg) {
    printk("echo: %s\n", msg.c_str());
    _return = msg;
  }

  int32_t counter() {
    ++count;
    printk("counter: %d\n", count);
    return count;
  }

protected:
  int count;
};
