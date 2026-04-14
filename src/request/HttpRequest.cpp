#include "HttpRequest.hpp"
#include "HttpMessage.hpp"
#include "Method.hpp"
#include "Status.hpp"
#include <cctype>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <utility>

#include <fcntl.h>
#include <unistd.h>

HttpRequest::HttpRequest()
    : HttpMessage(),
      m_uri(),
      m_method(method::UNKNOWN),
      m_complete(false)
{
}

// HttpRequest::HttpRequest(const HttpRequest &other)
//     : HttpMessage(other),
//       m_uri(other.m_uri),
//       m_method(other.m_method),
//       m_complete(other.m_complete)
// {
// }

// HttpRequest &HttpRequest::operator=(const HttpRequest &other)
// {
//     if (this == &other)
//         return *this;
//     this->operator=(other);
//     m_method   = other.m_method;
//     m_uri      = other.m_uri;
//     m_complete = other.m_complete;
//
//     return *this;
// }

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

HttpRequest::~HttpRequest()
{
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

bool HttpRequest::complete() const
{
    return m_complete;
}

void HttpRequest::setComplete(bool val)
{
    m_complete = val;
}

static const int WIDTH     = 42;
static const int KEY_WIDTH = 20;

static void print_top(const std::string &title)
{
    std::cout << "┌";
    int pad   = WIDTH - title.size();
    int left  = pad / 2;
    int right = pad - left;

    while (left > 0)
    {
        std::cout << "─";
        left--;
    }
    std::cout << " " << title << " ";
    while (right > 0)
    {
        std::cout << "─";
        right--;
    }
    std::cout << "┐\n";
}

static void print_separator(const std::string &title)
{
    std::cout << "├";
    int pad   = WIDTH - title.size();
    int left  = pad / 2;
    int right = pad - left;

    while (left > 0)
    {
        std::cout << "─";
        left--;
    }
    std::cout << " " << title << " ";
    while (right > 0)
    {
        std::cout << "─";
        right--;
    }
    std::cout << "┤\n";
}

static void print_row(const std::string &key, const std::string &value)
{
    std::cout << "│ " << std::left << std::setw(KEY_WIDTH) << key << "│ "
              << std::left << std::setw(WIDTH - KEY_WIDTH - 1) << value
              << "│\n";
}

static void print_row(const std::string &value)
{
    std::cout << "│" << std::left << std::setw(WIDTH + 2) << value << "│\n";
}

static void print_bottom()
{
    std::cout << "└";
    int left = WIDTH + 2;
    while (left > 0)
    {
        std::cout << "─";
        left--;
    }
    std::cout << "┘\n";
}

std::ostream &operator<<(std::ostream &os, const HttpRequest &request)
{
    std::ostringstream oss("HTTP/", std::_S_app);

    oss << request.version_major() << "." << request.version_minor();

    print_top("Request Line");
    print_row("Method", to_string(request.method()));
    print_row("Target", request.uri());
    print_row("Version", oss.str());
    print_row("", "");
    print_separator("Header Line");
    print_row("", "");
    HttpRequest::Headers::const_iterator it  = request.headers().begin();
    HttpRequest::Headers::const_iterator end = request.headers().end();
    while (it != end)
    {
        print_row(it->first, it->second);
        it++;
    }
    print_row("", "");
    print_separator("Body");
    std::ifstream ifs(request.body().c_path());

    char buffer[WIDTH];
    while (ifs.getline(buffer, WIDTH))
    {
        ifs.read(buffer, WIDTH);
        print_row(buffer);
    }
    print_bottom();

    return os;
}
