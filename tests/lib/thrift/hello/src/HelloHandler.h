#pragma once

#include <iostream>

#include "Hello.h"

class HelloHandler : virtual public HelloIf
{
public:
    HelloHandler() : count(0)
    {
    }

    void ping()
    {
        std::cout << "ping" << std::endl;
    }

    void echo(std::string &_return, const std::string &msg)
    {
        std::cout << "echo: " << msg << std::endl;
        _return = msg;
    }

    int32_t counter()
    {
        ++count;
        std::cout << "counter: " << count << std::endl;
        return count;
    }

protected:
    int count;
};
