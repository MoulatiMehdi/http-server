#include "HttpRequest.hpp"
#include "Status.hpp"
#include <cctype>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <utility>

#include <fcntl.h>
#include <unistd.h>

HttpRequest::HttpRequest()
    : HttpMessage(),
      m_target(),
      m_headers(),
      m_complete(false)
{
}

HttpRequest::HttpRequest(const HttpRequest &other)
    : HttpMessage(other),
      m_target(other.m_target),
      m_headers(other.m_headers),
      m_complete(other.m_complete)
{
}

HttpRequest &HttpRequest::operator=(const HttpRequest &other)
{
    if (this == &other)
        return *this;
    this->operator=(other);
    m_target   = other.m_target;
    m_headers  = other.m_headers;
    m_complete = other.m_complete;

    return *this;
}

HttpRequest::~HttpRequest()
{
}

void HttpRequest::setTarget(const std::string &uri)
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

bool HttpRequest::good() const
{
    return m_status == Status::ok;
}

HttpRequest::Headers::const_iterator
HttpRequest::get(const std::string &header_name) const
{
    return m_headers.find(header_name);
}

const HttpRequest::Headers &HttpRequest::headers() const
{
    return m_headers;
}

HttpRequest::Headers &HttpRequest::headers()
{
    return m_headers;
}

void HttpRequest::set(const std::string &name, const std::string &value)
{
    m_headers.insert(Headers::value_type(name, value));
}

bool HttpRequest::incomplete() const
{
    return !m_complete;
}

void HttpRequest::setComplete(bool val)
{
    m_complete = val;
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
        os << "\t" << "'" << it->first << "' : '" << it->second << "'"
           << std::endl;
        it++;
    }
    std::cout << "/********************* HTTP BODY "
                 "****************************/"
              << std::endl;

    int fd = open(request.body_file_name().c_str(), O_RDONLY);

    if (fd < 0)
    {
        std::perror("open");
        return os;
    }
    char    buff[1024];
    ssize_t size = 0;

    while ((size = read(fd, buff, 1024)) > 0)
    {
        std::cout.write(buff, size);
    }
    std::cout.flush();
    return os;
}
