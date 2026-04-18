#include "HttpRequest.hpp"
#include "HttpMessage.hpp"
#include "Method.hpp"
#include "Status.hpp"
#include <cctype>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <sys/types.h>

#include <fcntl.h>
#include <unistd.h>

HttpRequest::HttpRequest() : HttpMessage(), m_uri(), m_method(method::UNKNOWN)
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

bool HttpRequest::good() const
{
    return m_status == status::OK;
}

const HttpRequest::Headers &HttpRequest::headers() const
{
    return m_headers;
}

HttpRequest::Headers &HttpRequest::headers()
{
    return m_headers;
}

std::string HttpRequest::to_string() const
{
    std::ostringstream oss("");

    oss << "HTTP/" << version_major() << "." << version_minor() << " ";
    oss << status() << " " << phrase_reason(status()) << "\r\n";
    Headers::const_iterator begin = headers().begin();
    Headers::const_iterator end   = headers().end();
    while (begin != end)
    {
        oss << begin->first << ": " << begin->second << "\r\n";
        begin++;
    }
    oss << "\r\n";

    if (method() != method::POST)
        oss << "<discarded body (size : " << body().size() << ")>";
    else
    {
        int fd = open(body().c_path(), O_RDONLY);
        if (fd < 0)
        {
            perror("HttpRequest::to_string::open");
            return oss.str();
        }
        char buffer[1024];
        while (true)
        {
            ssize_t size = read(fd, buffer, 1024);
            if (size == 0)
                break;
            if (size < 0)
            {
                perror("read");
                break;
            }
            oss.write(buffer, size);
        }
    }
    return oss.str();
}

HttpRequest::~HttpRequest()
{
}
