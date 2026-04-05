#include "HttpRequest.hpp"
#include "Status.hpp"
#include <cctype>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>

HttpRequest::HttpRequest()
    : HttpMessage(),
      m_target(),
      m_content_length(0),
      m_status(Status::ok),
      m_headers()
{
}

HttpRequest::HttpRequest(const HttpRequest &other)
    : HttpMessage(other),
      m_target(other.m_target),
      m_content_length(other.m_content_length),
      m_status(other.m_status),
      m_headers(other.m_headers)
{
}

HttpRequest &HttpRequest::operator=(const HttpRequest &other)
{
    if (this == &other)
        return *this;
    this->operator=(other);
    m_target         = other.m_target;
    m_content_length = other.m_content_length;
    m_status         = other.m_status;
    m_headers        = other.m_headers;

    return *this;
}

HttpRequest::~HttpRequest()
{
}

void HttpRequest::set_target(const std::string &uri)
{
    m_target = uri;
}

const std::string &HttpRequest::target() const
{
    return m_target;
}

std::string &HttpRequest::target()
{
    return m_target;
}

const std::string &HttpRequest::body() const
{
    return m_body;
}

std::string &HttpRequest::body()
{
    return m_body;
}

bool HttpRequest::good() const
{
    return true;
}

const HttpRequest::Headers &HttpRequest::headers() const
{
    return m_headers;
}

HttpRequest::Headers &HttpRequest::headers()
{
    return m_headers;
}

void HttpRequest::setHeader(std::string &name, std::string &value)
{
    m_headers.insert(Headers::value_type(name, value));
}

std::ostream &operator<<(std::ostream &os, const HttpRequest &request)
{
    std::cout << "/********************* HTTP REQUEST LINE "
                 "****************************/"
              << std::endl;
    os << "\t" << request.method() << " " << request.target() << " " << "HTTP/"
       << request.version_major() << "." << request.version_minor()
       << std::endl;
    HttpRequest::Headers::const_iterator it  = request.headers().cbegin();
    HttpRequest::Headers::const_iterator end = request.headers().cend();
    std::cout << "/********************* HTTP HEADERS "
                 "****************************/"
              << std::endl;
    while (it != end)
    {
        os << "\t" << std::left << std::setw(32) << it->first << " : "
           << it->second << std::endl;
        it++;
    }
    std::cout << "/********************* HTTP BODY "
                 "****************************/"
              << std::endl;
    std::cout << request.body() << std::endl;
    return os;
}
