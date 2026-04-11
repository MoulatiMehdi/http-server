#include "ConfigParser.hpp"
#include <cstdlib>

std::string ConfigParser::parseListenValueToken() {
    advance();

    if (_tokens[_i].type != TOK_WORD)
        throwError("listen: expected address or port");

    return _tokens[_i].value;
}

ListenConfig ConfigParser::parseListenPortOnly(const std::string& value) {
    if (value.empty())
        throwError("listen: empty port");

    ListenConfig listen;
    listen.host = "0.0.0.0";
    listen.port = parseListenPort(value);
    return listen;
}

ListenConfig ConfigParser::parseListenHostPort(const std::string& value, std::size_t colonPos) {
    if (value.find(':', colonPos + 1) != std::string::npos)
        throwError("listen: invalid format");

    const std::string host = value.substr(0, colonPos);
    const std::string portStr = value.substr(colonPos + 1);

    if (host.empty())
        throwError("listen: host must not be empty");
    if (portStr.empty())
        throwError("listen: port must not be empty");

    if (!isValidIPv4(host) && !isValidHostname(host))
        throwError("listen: invalid host");

    ListenConfig listen;
    listen.host = host;
    listen.port = parseListenPort(portStr);
    return listen;
}

int ConfigParser::parseListenPort(const std::string& value) {
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(value[i])))
            throwError("listen: port must be numeric");
    }

    std::size_t port = std::strtoul(value.c_str(), NULL, 10);

    if (port > 65535)
        throwError("listen: port out of range");

    return static_cast<int>(port);
}
