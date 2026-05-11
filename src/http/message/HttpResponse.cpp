#include "HttpResponse.hpp"
#include "HttpMessage.hpp"
#include "Status.hpp"
#include "helper.hpp"
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>

HttpResponse::HttpResponse() : HttpMessage(), m_parser(*this)
{
}

HttpResponse::HttpResponse(Status status) : HttpMessage(status), m_parser(*this)
{
}

void HttpResponse::parse(const char *c_str, size_t len)
{
    m_parser.parse(c_str, len);
}

void HttpResponse::clear()
{
    HttpMessage::clear();
    m_parser.clear();
}

std::string HttpResponse::to_string()
{
    add_server_headers();

    static std::string names[] = {
        "date",
        "server",
        "content-type",
        "content-length",
        "content-encoding",
        "last-modified",
        "expires",
        "location",
        "www-authenticate",
        "allow",
        "pragma"
    };
    const size_t       size = sizeof(names) / sizeof(names[0]);
    std::ostringstream oss("");

    if (m_parser.m_code != 200)
        m_status = static_cast<Status>(m_parser.m_code);
    oss << "HTTP/" << version_major() << "." << version_minor() << " ";
    oss << status() << " " << phrase_reason(status()) << "\r\n";
    Headers::const_iterator it;
    if (getHeader("content-length") == m_headers.end())
        setHeader("content-length", toString(m_content_length));
    for (size_t i = 0; i < size; i++)
    {
        it = getHeader(names[i]);
        if (it != headers().end())
            oss << it->first << ": " << it->second << "\r\n";
    }
    HeadersRange range = m_headers.equal_range("set-cookie");
    for (Headers::const_iterator it = range.first; it != range.second; it++)
    {
        oss << it->first << ": " << it->second << "\r\n";
    }
    oss << "connection: close\r\n";
    oss << "\r\n";
    return oss.str();
}

size_t HttpResponse::gcount() const
{
    return m_parser.gcount();
}

HttpResponseParser &HttpResponse::parser()
{
    return m_parser;
}

std::string
HttpResponse::serve_directory(const std::string &root, std::string path)
{
    std::ostringstream oss;

    std::string pathname = root;
    if (root.empty() || *root.rbegin() != '/')
        pathname += "/";
    if (path.empty() || *path.rbegin() != '/')
        path += "/";
    pathname += path;

    DIR *fd = opendir(pathname.c_str());

    if (fd == NULL)
    {
        switch (errno)
        {
            case ENOTDIR:
            case ENOENT:
                m_status = status::NOT_FOUND;
                break;
            case EACCES:
                m_status = status::FORBIDDEN;
                break;
            default:
                m_status = status::INTERNAL_SERVER_ERROR;
        }
        return "";
    }
    struct dirent *dirval;

    oss << "<html>\n";
    oss << "<head><title>Index of " << path << "</title></head>";
    oss << "<body>\n";
    oss << "<h1>Index of " << path << "</h1><hr><pre><a href=\"../\">../</a>\n";

    struct stat file_stat;
    char        buff[64];

    while (true)
    {
        dirval = readdir(fd);
        if (dirval == NULL)
            break;

        std::memset(&file_stat, 0, sizeof(struct stat));
        std::memset(&file_stat, 0, sizeof(struct tm));

        std::string name(dirval->d_name);
        if (name == "." || name == "..")
            continue;

        stat(std::string(pathname + dirval->d_name).c_str(), &file_stat);
        std::strftime(
            buff, 64, "%d-%b-%Y %H:%M", localtime(&file_stat.st_mtim.tv_sec)
        );

        if (dirval->d_type == DT_DIR)
        {
            name += "/";
        }

        oss << "<a href=\"" << name << "\">";
        if (name.length() > 50)
            oss << name.substr(0, 47) << "..&gt;" << "</a>";
        else
            oss << name << "</a>" << std::string(50 - name.size(), ' ');

        oss << " " << buff << std::string(20, ' ');

        if (dirval->d_type != DT_DIR)
            oss << file_stat.st_size;
        else
            oss << "-";
        oss << "\n";
    }
    oss << "</pre><hr></body>\n";
    oss << "</html>\n";

    closedir(fd);
    setHeader("content-type", "text/html");
    m_content_length = oss.str().length();
    return to_string() + oss.str();
}

std::string HttpResponse::serve_page()
{
    std::ostringstream oss;
    StatusClass        status_class = to_status_class(m_status);
    std::string        bg;
    std::string        color;
    std::string        box_shadow;

    switch (status_class)
    {
        case status_class::UNKNOWN:
        case status_class::INFORMATIONAL:
            color      = "#111827;\n";
            bg         = "#4b5563;\n";
            box_shadow = "0 20px 50px rgba(17, 24, 39, 0.40);\n";
            break;

        case status_class::SUCCESSFUL:
            color      = "#166534;\n";
            bg         = "#15803d;\n";
            box_shadow = "0 20px 50px rgba(21, 128, 61, 0.40);\n";
            break;

        case status_class::REDIRECTION:
            color      = "#1e40af;\n";
            bg         = "#2563eb;\n";
            box_shadow = "0 20px 50px rgba(37, 99, 235, 0.40);\n";
            break;

        case status_class::CLIENT_ERROR:
            color      = "#b91c1c;\n";
            bg         = "#dc2626;\n";
            box_shadow = "0 20px 50px rgba(220, 38, 38, 0.40);\n";
            break;

        case status_class::SERVER_ERROR:
            color      = "#9f1239;\n";
            bg         = "#be123c;\n";
            box_shadow = "0 20px 50px rgba(190, 18, 60, 0.45);\n";
            break;
    }
    oss << "<!DOCTYPE html>\n";
    oss << "<html lang=\"en\">\n";
    oss << "<head>\n";
    oss << "<meta charset=\"UTF-8\">\n";
    oss << "<meta name=\"viewport\" content=\"width=device-width, "
           "initial-scale=1.0\">\n";
    oss << "<title>" << phrase_reason(m_status) << "</title>\n";
    oss << "\n";
    oss << "<style>\n";
    oss << "    * { box-sizing: border-box; }\n";
    oss << "\n";
    oss << "    body {\n";
    oss << "        margin: 0;\n";
    oss << "        height: 100vh;\n";
    oss << "        display: flex;\n";
    oss << "        align-items: center;\n";
    oss << "        justify-content: center;\n";
    oss << "        background: #f8fafc;\n";
    oss << "        font-family: -apple-system,system-ui,  sans-serif;\n";
    oss << "        color: #1e293b;\n";
    oss << "    }\n";
    oss << "\n";
    oss << "    .card {\n";
    oss << "        background: " << bg;
    oss << "        padding: 42px;\n";
    oss << "        border-radius: 18px;\n";
    oss << "        box-shadow:" << box_shadow;
    oss << "        max-width: 420px;\n";
    oss << "        width: 90%;\n";
    oss << "        text-align: center;\n";
    oss << "        color: #ffffff;\n";
    oss << "    }\n";
    oss << "\n";
    oss << "    .code {\n";
    oss << "        font-size: 64px;\n";
    oss << "        font-weight: 800;\n";
    oss << "        margin: 0;\n";
    oss << "        letter-spacing: 2px;\n";
    oss << "    }\n";
    oss << "\n";
    oss << "    .message {\n";
    oss << "        font-size: 22px;\n";
    oss << "        margin: 10px 0;\n";
    oss << "        font-weight: 500;\n";
    oss << "    }\n";
    oss << "\n";
    oss << "    .button {\n";
    oss << "        display: inline-block;\n";
    oss << "        padding: 10px 18px;\n";
    oss << "        border-radius: 8px;\n";
    oss << "        background: #ffffff;\n";
    oss << "        color:" << color;
    oss << "        text-decoration: none;\n";
    oss << "        font-weight: 800;\n";
    oss << "        transition: all 0.2s ease;\n";
    oss << "    }\n";
    oss << "\n";
    oss << "    .button:hover {\n";
    oss << "        background: #f1f5f9;\n";
    oss << "        transform: translateY(-1px);\n";
    oss << "    }\n";
    oss << "</style>\n";
    oss << "\n";
    oss << "</head>\n";
    oss << "\n";
    oss << "<body>\n";
    oss << "    <div class=\"card\">\n";
    oss << "        <p class=\"code\">" << m_status << "</p>\n";
    oss << "        <p class=\"message\">" << phrase_reason(m_status)
        << "</p>\n";
    oss << "        <a href=\"/\" class=\"button\">Go Home</a>\n";
    oss << "    </div>\n";
    oss << "</body>\n";
    oss << "</html>";
    setHeader("content-type", "text/html");
    m_content_length = oss.str().length();
    return to_string() + oss.str();
}

static std::string http_date()
{

    time_t now = std::time(NULL);
    tm    *gmt = std::localtime(&now);

    // format according to RFC 1123
    char buf[64];
    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", gmt);

    return std::string(buf);
}

void HttpResponse::add_server_headers()
{
    setHeader("server", "webserv/1.0");
    setHeader("date", http_date());
}

HttpResponse::~HttpResponse()
{
    m_body.close();
    m_body.remove();
}
