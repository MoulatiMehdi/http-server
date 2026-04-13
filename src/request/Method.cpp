#include "Method.hpp"
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>

Method string_to_method(const std::string &str)
{
    if (str == "GET")
        return method::GET;
    else if (str == "POST")
        return method::POST;
    else if (str == "DELETE")
        return method::DELETE;
    return method::UNKNOWN;
}

const std::string to_string(Method method)
{
    switch (method)
    {
        case method::GET:
            return "GET";
        case method::DELETE:
            return "DELETE";
        case method::POST:
            return "POST";
        case method::UNKNOWN:
            return "<unknown>";
    }
    throw std::invalid_argument("invalid argument");
}

std::ostream &operator<<(std::ostream &os, const Method &method)
{
    os << to_string(method);
    return os;
}
