#include "HttpRequest.hpp"
#include "Config.hpp"
#include "HttpMessage.hpp"
#include "HttpRequestParser.hpp"
#include "Method.hpp"
#include <cctype>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <sys/types.h>

#include <fcntl.h>
#include <unistd.h>

HttpRequest::HttpRequest(const ServerConfig &config)
    : HttpMessage(),
      m_uri(),
      m_method(method::UNKNOWN),
      m_parser(*this),
      config(config)
{
}

Method HttpRequest::method() const
{
    return m_method;
}

void HttpRequest::setMethod(Method method)
{
    m_method = method;
}

void HttpRequest::setMethod(std::string &method)
{
    m_method = string_to_method(method);
}

void HttpRequest::setUri(const std::string &uri)
{
    m_uri = uri;
}

const std::string &HttpRequest::uri() const
{
    return m_uri;
}

std::string &HttpRequest::uri()
{
    return m_uri;
}

void HttpRequest::parse(const char *c_str, size_t len)
{
    m_parser.parse(c_str, len);
}

HttpRequestParser &HttpRequest::parser()
{
    return m_parser;
}

std::string HttpRequest::to_string() const
{
    std::ostringstream oss("");

    oss << m_method << " " << m_uri << " HTTP/" << version_major() << "."
        << version_minor() << " \n";
    return oss.str();
}

HttpRequest::~HttpRequest()
{
}
