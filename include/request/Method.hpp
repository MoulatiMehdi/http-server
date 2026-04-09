#ifndef METHOD_HPP
#define METHOD_HPP

#include <string>

namespace method
{
    enum Method
    {
        Delete,
        Get,
        Post,
        Unknown
    };

} // namespace method

//
using method::Method;

const std::string to_string(Method method);
Method            string_to_method(const std::string &str);

std::ostream &operator<<(std::ostream &os, const Method &method);
std::istream &operator>>(std::istream &os, Method &method);

#endif
