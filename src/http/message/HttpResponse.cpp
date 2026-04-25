#include "HttpResponse.hpp"
#include "HttpMessage.hpp"
#include "Status.hpp"
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

HttpResponse::HttpResponse() : HttpMessage(), m_parser(*this)
{
    setHeader("Server", "webserv");
    setHeader("Connection", "close");
}

HttpResponse::HttpResponse(Status status) : HttpMessage(status), m_parser(*this)
{
    setHeader("Server", "webserv");
    setHeader("Connection", "close");
}

void HttpResponse::parse(const char *c_str, size_t len)
{
    m_parser.parse(c_str, len);
}

std::string HttpResponse::to_string() const
{
    std::ostringstream oss("");

    oss << "HTTP/" << version_major() << "." << version_minor() << " ";
    oss << status() << " " << phrase_reason(status()) << "\r\n";
    Headers::const_iterator begin = headers().begin();
    Headers::const_iterator end   = headers().end();
    oss << "content-length: " << m_content_length << "\r\n";
    while (begin != end)
    {
        if (begin->first != "content-length")
            oss << begin->first << ": " << begin->second << "\r\n";
        begin++;
    }
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

    oss << "<!DOCTYPE html>\n<html>\n";
    oss << "  <head>\n";
    oss << "<link rel=\"preconnect\" href=\"https://fonts.googleapis.com\">";
    oss << "<link rel=\"preconnect\" href=\"https://fonts.gstatic.com\" "
           "crossorigin>";
    oss << "<link "
           "href=\"https://fonts.googleapis.com/"
           "css2?family=Nunito:ital,wght@0,200..1000;1,200..1000&display="
           "swap\" rel=\"stylesheet\">";
    oss << "<link rel=\"preconnect\" href=\"https://fonts.googleapis.com\">";
    oss << "<link rel=\"preconnect\" href=\"https://fonts.gstatic.com\" "
           "crossorigin>";
    oss << "<link "
           "href=\"https://fonts.googleapis.com/"
           "css2?family=Nunito:ital,wght@0,200..1000;1,200..1000&family=Roboto:"
           "ital,wght@0,100..900;1,100..900&display=swap\" rel=\"stylesheet\">";
    oss << "<meta charset=\"UTF-8\">";
    oss << "<style type=\"text/css\">";
    oss << "html {";
    oss << "  font-family: \"Nunito\" ,sans-serif;";
    oss << "}";
    oss << "img {";
    oss << "  border: 0;";
    oss << "}";
    oss << "th {";
    oss << "  font-family: \"Roboto\",sans-serif;";
    oss << "  text-align: start;";
    oss << "  white-space: nowrap;";
    oss << "}";
    oss << "th > a {";
    // oss << "font-family: \"Nunito\" , sans-serif;";
    oss << "  color: inherit;";
    oss << "}";
    oss << "table[order] > thead > tr > th {";
    oss << "  cursor: pointer;";
    oss << "}";
    oss << "table[order] > thead > tr > th::after {";
    oss << "  display: none;";
    oss << "  width: .8em;";
    oss << "  margin-inline-end: -.8em;";
    oss << "  text-align: end;";
    oss << "}";
    oss << "table[order=\"asc\"] > thead > tr > th::after {";
    oss << "  content: \"\\2193\"; /* DOWNWARDS ARROW (U+2193) */";
    oss << "}";
    oss << "table[order=\"desc\"] > thead > tr > th::after {";
    oss << "  content: \"\\2191\"; /* UPWARDS ARROW (U+2191) */";
    oss << "}";
    oss << "table[order][order-by=\"0\"] > thead > tr > th:first-child > a ,";
    oss << "table[order][order-by=\"1\"] > thead > tr > th:first-child + th > "
           "a ,";
    oss << "table[order][order-by=\"2\"] > thead > tr > th:first-child + th + "
           "th > a {";
    oss << "  text-decoration: underline;";
    oss << "}";
    oss << "table[order][order-by=\"0\"] > thead > tr > th:first-child::after "
           ",";
    oss << "table[order][order-by=\"1\"] > thead > tr > th:first-child + "
           "th::after ,";
    oss << "table[order][order-by=\"2\"] > thead > tr > th:first-child + th + "
           "th::after {";
    oss << "  display: inline-block;";
    oss << "}";
    oss << "table.remove-hidden > tbody > tr.hidden-object {";
    oss << "  display: none;";
    oss << "}";
    oss << "td {";
    oss << "  font-family: \"Nunito\", sans-serif;";
    oss << "  white-space: nowrap;";
    oss << "}";
    oss << "table.ellipsis {";
    oss << "  width: 100%;";
    oss << "  table-layout: fixed;";
    oss << "  border-spacing: 0;";
    oss << "}";
    oss << "table.ellipsis > tbody > tr > td {";
    oss << "  overflow: hidden;";
    oss << "  text-overflow: ellipsis;";
    oss << "}";
    oss << "/* name */";
    oss << "/* name */";
    oss << "th:first-child {";
    oss << "  padding-inline-end: 2em;";
    oss << "}";
    oss << "/* size */";
    oss << "th:first-child + th {";
    oss << "  padding-inline-end: 1em;";
    oss << "}";
    oss << "td:first-child + td {";
    oss << "  text-align: end;";
    oss << "  padding-inline-end: 1em;";
    oss << "}";
    oss << "/* date */";
    oss << "td:first-child + td + td {";
    oss << "  padding-inline-start: 1em;";
    oss << "  padding-inline-end: .5em;";
    oss << "}";
    oss << "/* time */";
    oss << "td:first-child + td + td + td {";
    oss << "  padding-inline-start: .5em;";
    oss << "}";
    oss << ".symlink {";
    oss << "  font-style: italic;";
    oss << "}";
    oss << ".dir ,";
    oss << ".symlink ,";
    oss << ".file {";
    oss << "  margin-inline-start: 20px;";
    oss << "}";
    oss << ".dir::before ,";
    oss << ".file > img {";
    oss << "  margin-inline-end: 4px;";
    oss << "  margin-inline-start: -20px;";
    oss << "  width: 16px;";
    oss << "  height: 16px;";
    oss << "  vertical-align: middle;";
    oss << "}";
    oss << ".dir::before {";
    oss << "  content: "
           "url(https://cdn-icons-png.flaticon.com/512/3735/3735057.png);";
    oss << "}";
    oss << "</style>";
    oss << "<link rel=\"stylesheet\" media=\"screen, projection\" "
           "type=\"text/css\" "
           "href=\"chrome://global/skin/dirListing/dirListing.css\">";
    oss << "<script type=\"application/javascript\">";
    oss << "'use strict';";
    oss << "var gTable, gOrderBy, gTBody, gRows, gUI_showHidden;";
    oss << "document.addEventListener(\"DOMContentLoaded\", function() {";
    oss << "  gTable = document.getElementsByTagName(\"table\")[0];";
    oss << "  gTBody = gTable.tBodies[0];";
    oss << "  if (gTBody.rows.length < 2)";
    oss << "    return;";
    oss << "  gUI_showHidden = document.getElementById(\"UI_showHidden\");";
    oss << "  var headCells = gTable.tHead.rows[0].cells,";
    oss << "      hiddenObjects = false;";
    oss << "  function rowAction(i) {";
    oss << "    return function(event) {";
    oss << "      event.preventDefault();";
    oss << "      orderBy(i);";
    oss << "    }";
    oss << "  }";
    oss << "  for (var i = headCells.length - 1; i >= 0; i--) {";
    oss << "    var anchor = document.createElement(\"a\");";
    oss << "    anchor.href = \"\";";
    oss << "    anchor.appendChild(headCells[i].firstChild);";
    oss << "    headCells[i].appendChild(anchor);";
    oss << "    headCells[i].addEventListener(\"click\", rowAction(i), true);";
    oss << "  }";
    oss << "  if (gUI_showHidden) {";
    oss << "    gRows = Array.from(gTBody.rows);";
    oss << "    hiddenObjects = gRows.some(row => row.className == "
           "\"hidden-object\");";
    oss << "  }";
    oss << "  gTable.setAttribute(\"order\", \"\");";
    oss << "  if (hiddenObjects) {";
    oss << "    gUI_showHidden.style.display = \"block\";";
    oss << "    updateHidden();";
    oss << "  }";
    oss << "}, \"false\");";
    oss << "function compareRows(rowA, rowB) {";
    oss << "  var a = rowA.cells[gOrderBy].getAttribute(\"sortable-data\") || "
           "\"\";";
    oss << "  var b = rowB.cells[gOrderBy].getAttribute(\"sortable-data\") || "
           "\"\";";
    oss << "  var intA = +a;";
    oss << "  var intB = +b;";
    oss << "  if (a == intA && b == intB) {";
    oss << "    a = intA;";
    oss << "    b = intB;";
    oss << "  } else {";
    oss << "    a = a.toLowerCase();";
    oss << "    b = b.toLowerCase();";
    oss << "  }";
    oss << "  if (a < b)";
    oss << "    return -1;";
    oss << "  if (a > b)";
    oss << "    return 1;";
    oss << "  return 0;";
    oss << "}";
    oss << "function orderBy(column) {";
    oss << "  if (!gRows)";
    oss << "    gRows = Array.from(gTBody.rows);";
    oss << "  var order;";
    oss << "  if (gOrderBy == column) {";
    oss << "    order = gTable.getAttribute(\"order\") == \"asc\" ? \"desc\" : "
           "\"asc\";";
    oss << "  } else {";
    oss << "    order = \"asc\";";
    oss << "    gOrderBy = column;";
    oss << "    gTable.setAttribute(\"order-by\", column);";
    oss << "    gRows.sort(compareRows);";
    oss << "  }";
    oss << "  gTable.removeChild(gTBody);";
    oss << "  gTable.setAttribute(\"order\", order);";
    oss << "  if (order == \"asc\")";
    oss << "    for (var i = 0; i < gRows.length; i++)";
    oss << "      gTBody.appendChild(gRows[i]);";
    oss << "  else";
    oss << "    for (var i = gRows.length - 1; i >= 0; i--)";
    oss << "      gTBody.appendChild(gRows[i]);";
    oss << "  gTable.appendChild(gTBody);";
    oss << "}";
    oss << "function updateHidden() {";
    oss << "  gTable.className = "
           "gUI_showHidden.getElementsByTagName(\"input\")[0].checked ?";
    oss << "                     \"\" :";
    oss << "                     \"remove-hidden\";";
    oss << "}";
    oss << "</script>";
    oss << "<link rel=\"icon\" type=\"image/png\" "
           "href=\"data:image/"
           "png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8%"
           "2F9hAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAjFJREFUeNq"
           "sU8uOElEQPffR3XQ3ONASdBJCSBxHos5%2B3Bg3rvkCv8PElS78gPkO%"
           "2FATjQoUdO2ftrJiRh6aneTb9sOpC4weMN6lcuFV16pxDIfI8x12OYIDhcPiu2Wx%"
           "2B%2FHF5CW1Z6Jyegt%2FTNEWSJIjjGFEUIQxDrFYrWFSzXC4%2FdLvd95pRKpXKy%"
           "2BpRFZ7nwaWo1%"
           "2BsGnQG2260BKJfLKJVKGI1GEEJw7ateryd0v993W63WEwjgxfn5obGYzgCbzcaEbds"
           "IggDj8Riu6z6iUk9SYZMSx8W0LMsM%"
           "2FSKK75xnJlIq80anQXdbEp0OhcPJ0eiaJnGRMEyyPDsAKKUM9clkYoDo3SZJzzSdp0"
           "VSKYmfV1co%2Bz580kw5KD<!DOCTYPE "
           "html>IM8RbRfEnUf1HzxtQyMAGcaGruTKczMzEIaqhKifV6jd%"
           "2BzGQQB5llunF%"
           "2FM52BizC2K5sYPYvZcu653tjOM9O93wnYc08gmkgg4VAxixfqFUJT36AYBZGd6PJkF"
           "CZnnlBxMp38gqIgLpZB0y4Nph18lyWh5FFbrOSxbl3V4G%2BVB7T4ajYYxTyuLtO%"
           "2BCvWGgJE1Mc7JNsJEhvgw%2FQV4fo%2F24nbEsX2u1d5sVyn8sJO0ZAQiIYnFh%"
           "2BxrfLz%2Fj29cBS%2FO14zg3i8XigW3ZkErDtmKoeM%"
           "2BAJGRMnXeEPGKf0nCD1ydvkDzU9Jbc6OpR7WIw6L8lQ%2B4pQ1%"
           "2FlPF0RGM9Ns91Wmptk0GfB4EJkt77vXYj%2F8m%2B8y%"
           "2FkrwABHbz2H9V68DQAAAABJRU5ErkJggg%3D%3D\">";
    oss << "<title>Index of " << path << "</title>";
    oss << "</head>";
    oss << "  <body>\n";
    oss << "    <h1 style=\"font-family: Roboto , sans-serif;font-weight: "
           "bold; padding-bottom : 10px;\">Index of file "
        << path << "</h1>";
    oss << "    <table order=\"asc\" order-by=\"0\">\n";
    oss << "     <thead>\n";
    oss << "      <tr>\n";
    oss << "        <th>Name</th>\n";
    oss << "        <th>Size</th>\n";
    oss << "        <th colspan=\"2\" >Last Modified</th>\n";
    oss << "      </tr>\n";
    oss << "     <thead>\n";
    oss << "     <tbody>\n";

    struct tm   tm_info;
    struct stat file_stat;
    char        buff[64];

    while (true)
    {
        dirval = readdir(fd);
        if (dirval == NULL)
            break;

        memset(&file_stat, 0, sizeof(struct stat));
        memset(&file_stat, 0, sizeof(struct tm));
        stat(std::string(pathname + dirval->d_name).c_str(), &file_stat);
        gmtime_r(&file_stat.st_mtim.tv_sec, &tm_info); // or localtime_r
        std::strftime(buff, 64, "%Y-%m-%d %H:%M:%S GMT", &tm_info);

        oss << "<tr>\n";
        if (dirval->d_type == DT_DIR)
        {
            oss << "      <td sortable-data=\"1" << dirval->d_name << "\" > ";
            oss << "<table class=\"ellipsis\">";
            oss << "<tbody>";
            oss << "    <tr>";
            oss << "      <td>";
            oss << "<a href=\"" << path << dirval->d_name
                << "\" class=\"dir\" > ";
            oss << dirval->d_name << " </a>";
            oss << "      </td>";
            oss << "    </tr>";
            oss << "  </tbody>";
            oss << "  </table>";
            oss << "</td>\n";
            oss << "<td></td>\n";
        }
        else
        {
            oss << "      <td style=\"text-align: left;\" sortable-data=\"2"
                << dirval->d_name << "\" > ";
            oss << "<a href=\"" << path << dirval->d_name
                << "\"  class=\"file\"> ";
            oss << "<img "
                   "src=\"https://cdn-icons-png.flaticon.com/512/4904/"
                   "4904193.png\" >";
            oss << dirval->d_name << " </a></td>\n";
            oss << "<td style=\"text-align: center;\" storable-data=\""
                << file_stat.st_size << "\"> "
                << std::ceil(file_stat.st_size / 1024) << "KB</td>";
        }

        oss << "<td style=\"text-align: center;\" colspan=\"2\" "
               "storable-data=\""
            << file_stat.st_mtim.tv_sec << "\">" << buff << "</td>";
        oss << "</tr>\n";
    }
    oss << "     </tbody>\n";
    oss << "    </table>\n";
    oss << "  </body>\n";
    oss << "</html>\n";

    closedir(fd);
    setHeader("content-type", "text/html");
    m_content_length = oss.str().length();
    return to_string() + oss.str();
}

std::string HttpResponse::serve_page()
{
    std::ostringstream oss;
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
    oss << "        background: #f8fafc;\n"; // soft white (better than pure
                                             // white)
    oss << "        font-family: system-ui, -apple-system, sans-serif;\n";
    oss << "        color: #1e293b;\n";      // dark slate text
    oss << "    }\n";
    oss << "\n";
    oss << "    .card {\n";
    oss << "        background: linear-gradient(135deg, #ef4444, #b91c1c);\n";
    oss << "        padding: 42px;\n";
    oss << "        border-radius: 18px;\n";
    oss << "        box-shadow: 0 20px 50px rgba(185, 28, 28, 0.35);\n";
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
    oss << "    .detail {\n";
    oss << "        font-size: 14px;\n";
    oss << "        color: rgba(255,255,255,0.85);\n";
    oss << "        margin-bottom: 26px;\n";
    oss << "    }\n";
    oss << "\n";
    oss << "    .button {\n";
    oss << "        display: inline-block;\n";
    oss << "        padding: 10px 18px;\n";
    oss << "        border-radius: 8px;\n";
    oss << "        background: #ffffff;\n";
    oss << "        color: #b91c1c;\n"; // matches card
    oss << "        text-decoration: none;\n";
    oss << "        font-weight: 600;\n";
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

HttpResponse::~HttpResponse()
{
}
