#include "HttpResponse.hpp"
#include <cctype>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sys/types.h>

#include <fcntl.h>
#include <unistd.h>

HttpResponse::HttpResponse() : HttpMessage(), m_complete(false)
{
}

HttpResponse::HttpResponse(const HttpResponse &other)
    : HttpMessage(other),
      m_complete(other.m_complete)
{
}

HttpResponse &HttpResponse::operator=(const HttpResponse &other)
{
    if (this == &other)
        return *this;
    this->operator=(other);
    m_complete = other.m_complete;

    return *this;
}

HttpResponse::~HttpResponse()
{
}

bool HttpResponse::good() const
{
    return m_status == Status::OK;
}

bool HttpResponse::complete() const
{
    return m_complete;
}

void HttpResponse::setComplete(bool val)
{
    m_complete = val;
}

std::ostream &operator<<(std::ostream &os, const HttpResponse &response)
{
    std::cout << "/********************* HTTP response LINE "
                 "****************************/"
              << std::endl;
    os  << "HTTP/" << response.version_major() << "."
       << response.version_minor() << " " << response.status() << std::endl;
    HttpResponse::Headers::const_iterator it = response.headers().cbegin();
    HttpResponse::Headers::const_iterator end = response.headers().cend();
    std::cout << "/********************* HTTP HEADERS "
                 "****************************/"
              << std::endl;
    while (it != end)
    {
        os  << "'" << it->first << "' : '" << it->second << "'"
           << std::endl;
        it++;
    }
    std::cout << "/********************* HTTP BODY "
                 "****************************/"
              << std::endl;

    int fd = open(response.body().c_path(), O_RDONLY);

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
