#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Status.hpp"
#include <cstddef>
#include <cstdio>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <unistd.h>

const std::string ascii_repr[256] = {
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
    "\\",
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
    "\\x7F",
    "\\x80",
    "\\x81",
    "\\x82",
    "\\x83",
    "\\x84",
    "\\x85",
    "\\x86",
    "\\x87",
    "\\x88",
    "\\x89",
    "\\x8A",
    "\\x8B",
    "\\x8C",
    "\\x8D",
    "\\x8E",
    "\\x8F",
    "\\x90",
    "\\x91",
    "\\x92",
    "\\x93",
    "\\x94",
    "\\x95",
    "\\x96",
    "\\x97",
    "\\x98",
    "\\x99",
    "\\x9A",
    "\\x9B",
    "\\x9C",
    "\\x9D",
    "\\x9E",
    "\\x9F",
    "\\xA0",
    "\\xA1",
    "\\xA2",
    "\\xA3",
    "\\xA4",
    "\\xA5",
    "\\xA6",
    "\\xA7",
    "\\xA8",
    "\\xA9",
    "\\xAA",
    "\\xAB",
    "\\xAC",
    "\\xAD",
    "\\xAE",
    "\\xAF",
    "\\xB0",
    "\\xB1",
    "\\xB2",
    "\\xB3",
    "\\xB4",
    "\\xB5",
    "\\xB6",
    "\\xB7",
    "\\xB8",
    "\\xB9",
    "\\xBA",
    "\\xBB",
    "\\xBC",
    "\\xBD",
    "\\xBE",
    "\\xBF",
    "\\xC0",
    "\\xC1",
    "\\xC2",
    "\\xC3",
    "\\xC4",
    "\\xC5",
    "\\xC6",
    "\\xC7",
    "\\xC8",
    "\\xC9",
    "\\xCA",
    "\\xCB",
    "\\xCC",
    "\\xCD",
    "\\xCE",
    "\\xCF",
    "\\xD0",
    "\\xD1",
    "\\xD2",
    "\\xD3",
    "\\xD4",
    "\\xD5",
    "\\xD6",
    "\\xD7",
    "\\xD8",
    "\\xD9",
    "\\xDA",
    "\\xDB",
    "\\xDC",
    "\\xDD",
    "\\xDE",
    "\\xDF",
    "\\xE0",
    "\\xE1",
    "\\xE2",
    "\\xE3",
    "\\xE4",
    "\\xE5",
    "\\xE6",
    "\\xE7",
    "\\xE8",
    "\\xE9",
    "\\xEA",
    "\\xEB",
    "\\xEC",
    "\\xED",
    "\\xEE",
    "\\xEF",
    "\\xF0",
    "\\xF1",
    "\\xF2",
    "\\xF3",
    "\\xF4",
    "\\xF5",
    "\\xF6",
    "\\xF7",
    "\\xF8",
    "\\xF9",
    "\\xFA",
    "\\xFB",
    "\\xFC",
    "\\xFD",
    "\\xFE",
    "\\xFF",
};

const std::string &get_astring(unsigned char c)
{
    return ascii_repr[c];
}

static const int WIDTH     = 110;
static const int KEY_WIDTH = 30;

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
        std::cout << get_astring(value[i]);
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

void print_request(const HttpRequest &request)
{
    std::ostringstream oss;

    oss.clear();
    oss.str("");
    oss << "HTTP/" << request.version_major() << "." << request.version_minor();

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

    int fd = open(request.body().c_path(), O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return;
    }

    char buffer[WIDTH];
    while (true)
    {
        ssize_t size = read(fd, buffer, WIDTH);
        if (size == 0)
            break;
        if (size < 0)
        {
            perror("read");
            break;
        }
        std::string s;
        s.assign(buffer, size);
        print_row(s);
    }
    print_bottom();
}

void print_response(const HttpResponse &response)
{
    std::ostringstream oss("HTTP/", std::_S_app);

    oss << response.version_major() << "." << response.version_minor();

    print_top("response Line");
    print_row("Version", oss.str());
    oss.clear();
    oss.str("");
    oss << response.status() << " " << phrase_reason(response.status());
    print_row("Status", oss.str());
    print_row("", "");
    print_separator("Header Line");
    print_row("", "");
    HttpMessage::Headers::const_iterator it  = response.headers().begin();
    HttpMessage::Headers::const_iterator end = response.headers().end();
    while (it != end)
    {
        print_row(it->first, it->second);
        it++;
    }
    print_row("", "");
    print_separator("Body");
    std::ifstream ifs(response.body().c_path());

    char buffer[WIDTH];
    while (ifs.getline(buffer, WIDTH))
    {
        ifs.read(buffer, WIDTH);
        print_row(buffer);
    }
    print_bottom();
}

void print_string(const std::string &value)
{

    for (size_t i = 0; i < value.size(); i++)
    {
        std::cout << get_astring(value[i]);
    }
}

void print_string_nl(const std::string &value)
{

    for (size_t i = 0; i < value.size(); i++)
    {
        std::cout << get_astring(value[i]);
        if (value[i] == '\n')
            std::cout << std::endl;
    }
}

void print_ptr_nl(char *value, size_t len)
{

    for (size_t i = 0; i < len; i++)
    {
        std::cout << get_astring(value[i]);
        if (value[i] == '\n')
            std::cout << std::endl;
    }
}
