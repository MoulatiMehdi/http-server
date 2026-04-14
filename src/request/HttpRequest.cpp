#include "HttpRequest.hpp"
#include "HttpMessage.hpp"
#include "Method.hpp"
#include "Status.hpp"
#include <cctype>
#include <cstddef>
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

const char *ascii_repr[128] = {
    "\\0",
    "\\x01",
    "\\x02",
    "\\x03",
    "\\x04",
    "\\x05",
    "\\x06",
    "\\a",
    "\\b",
    "\\t",
    "\\n",
    "\\v",
    "\\f",
    "\\r",
    "\\x0E",
    "\\x0F",
    "\\x10",
    "\\x11",
    "\\x12",
    "\\x13",
    "\\x14",
    "\\x15",
    "\\x16",
    "\\x17",
    "\\x18",
    "\\x19",
    "\\x1A",
    "\\x1B",
    "\\x1C",
    "\\x1D",
    "\\x1E",
    "\\x1F",
    " ",
    "!",
    "\"",
    "#",
    "$",
    "%",
    "&",
    "'",
    "(",
    ")",
    "*",
    "+",
    ",",
    "-",
    ".",
    "/",
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    ":",
    ";",
    "<",
    "=",
    ">",
    "?",
    "@",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "[",
    "\\\\",
    "]",
    "^",
    "_",
    "`",
    "a",
    "b",
    "c",
    "d",
    "e",
    "f",
    "g",
    "h",
    "i",
    "j",
    "k",
    "l",
    "m",
    "n",
    "o",
    "p",
    "q",
    "r",
    "s",
    "t",
    "u",
    "v",
    "w",
    "x",
    "y",
    "z",
    "{",
    "|",
    "}",
    "~",
    "\\x7F"
};

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
    m_complete = m_status == status::OK && val;
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
    std::cout << "│ ";

    for (size_t i = 0; i < value.size(); i++)
    {
        std::cout.write(ascii_repr[(int)value[i]], 1);
    }
    for (size_t i = value.size(); i < WIDTH; i++)
    {
        std::cout.write(" ", 1);
    }
    std::cout << " │\n";
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
