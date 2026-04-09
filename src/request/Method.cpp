#include "Method.hpp"
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>

Method string_to_method(const std::string &str)
{
    if (str == "GET")
        return Method::GET;
    else if (str == "POST")
        return Method::POST;
    else if (str == "DELETE")
        return Method::DELETE;
    return Method::UNKNOWN;
}

const std::string to_string(Method method)
{
    switch (method)
    {
        case Method::GET:
            return "GET";
        case Method::DELETE:
            return "DELETE";
        case Method::POST:
            return "POST";
        case Method::UNKNOWN:
            return "<unknown>";
    }
    throw std::invalid_argument("invalid argument");
}

std::ostream &operator<<(std::ostream &os, const Method &method)
{
    os << to_string(method);
    return os;
}
