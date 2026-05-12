#include "HttpRequest.hpp"
#include "Config.hpp"
#include "HttpMessage.hpp"
#include "HttpRequestParser.hpp"
#include "Method.hpp"
#include "Uri.hpp"
#include <cctype>
#include <cstddef>
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
      m_max_body_size(0),
      m_server_config(config)
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
    m_uri.setUri(uri);
}

const Uri &HttpRequest::uri() const
{
    return m_uri;
}

Uri &HttpRequest::uri()
{
    return m_uri;
}

void HttpRequest::parse(const char *c_str, size_t len)
{
    m_parser.parse(c_str, len);
}

const HttpRequestParser &HttpRequest::parser() const
{
    return m_parser;
}

HttpRequestParser &HttpRequest::parser() 
{
    return m_parser;
}
void HttpRequest::setMaxBodySize(size_t size)
{
    m_max_body_size = size;
}

size_t HttpRequest::maxBodySize() const
{
    return m_max_body_size;
}

std::string HttpRequest::to_string() const
{
    std::ostringstream oss("");

    oss << m_method << " " << m_uri.origin() << " HTTP/" << version_major()
        << "." << version_minor() << "\n";
    Headers::const_iterator it = m_headers.begin();
    while (it != m_headers.end())
    {
        oss << it->first << ": " << it->second << "\n";
        it++;
    }
    return oss.str();
}

HttpRequest::~HttpRequest()
{
    m_body.close();
    m_body.remove();
}
