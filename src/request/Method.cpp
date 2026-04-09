#include "Method.hpp"
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>

Method string_to_method(const std::string &str)
{
    if (str == "GET")
        return method::Get;
    else if (str == "POST")
        return method::Post;
    else if (str == "DELETE")
        return method::Delete;
    return method::Unknown;
}

const std::string to_string(Method method)
{
    switch (method)
    {
        case method::Get:
            return "GET";
        case method::Delete:
            return "DELETE";
        case method::Post:
            return "POST";
        case method::Unknown:
            return "<unknown>";
    }
    throw std::invalid_argument("invalid argument");
}

std::ostream &operator<<(std::ostream &os, const Method &method)
{
    os << to_string(method);
    return os;
}
