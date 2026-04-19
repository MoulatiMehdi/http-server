#include "HttpResponse.hpp"
#include "HttpMessage.hpp"
#include "Status.hpp"
#include <cctype>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <sys/types.h>

#include <fcntl.h>
#include <unistd.h>

HttpResponse::HttpResponse() : HttpMessage()
{
}

std::string HttpResponse::to_string() const
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
    return oss.str();
}

HttpResponse::~HttpResponse()
{
}
