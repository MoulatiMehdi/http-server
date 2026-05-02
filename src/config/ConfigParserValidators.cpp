#include "ConfigParser.hpp"
#include <cstdlib>
#include <limits>

bool ConfigParser::isValidIPv4(const std::string& ip) {
    std::stringstream ss(ip);
    std::string part;
    std::size_t count = 0;

    while (std::getline(ss, part, '.')) {
        if (part.empty() || part.size() > 3)
            return false;

        for (std::size_t i = 0; i < part.size(); i++)
            if (!std::isdigit(part[i]))
                return false;

        int num = std::atoi(part.c_str());
        if (num > 255)
            return false;

        count++;
    }
    return count == 4;
}

bool ConfigParser::isValidHostname(const std::string& host) {
    if (host.empty())
        return false;

    if (host[0] == '.' || host[0] == '-' ||
        host[host.size() - 1] == '.' || host[host.size() - 1] == '-')
        return false;

    for (size_t i = 0; i < host.size(); i++) {
        char c = host[i];

        if (!(std::isalnum(c) || c == '-' || c == '.'))
            return false;

        if (c == '.' && i + 1 < host.size() && host[i + 1] == '.')
            return false;
    }

    return true;
}

std::size_t ConfigParser::parseSize(const std::string& raw)
{
    for (std::size_t i = 0; i < raw.size() - 1; i++)
        if (!std::isdigit(raw[i]))
            throwError("client_max_body_size: invalid value '" + raw + "'");

    char suffix = std::tolower(raw[raw.size() - 1]);
    std::size_t multiplier = 1;

    if (std::isdigit(suffix))
        multiplier = 1;
    else if (suffix == 'k')
        multiplier = 1024;
    else if (suffix == 'm')
        multiplier = 1024 * 1024;
    else if (suffix == 'g')
        multiplier = 1024 * 1024 * 1024;
    else
        throwError("client_max_body_size: unknown suffix '" + std::string(1, suffix) + "'");

    std::string numPart = std::isdigit(suffix) ? raw : raw.substr(0, raw.size() - 1);
    std::size_t value = std::strtoul(numPart.c_str(), NULL, 10);

    if (value > std::numeric_limits<std::size_t>::max() / multiplier)
        throwError("client_max_body_size: value too large");

    return value * multiplier;
}

bool ConfigParser::isAllDigit(const std::string& num) {
    for (std::size_t i = 0; i < num.size(); i++)
        if (!std::isdigit(num[i]))
            return false;
    return true;
}

bool ConfigParser::isValidPath(const std::string& path) {
    return !path.empty() && path[0] && path[path.size() - 1] == '/';
}

