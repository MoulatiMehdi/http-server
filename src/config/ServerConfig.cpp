#include "Config.hpp"
#include "MimeType.hpp"

MimeType ServerConfig::mimetype;

std::string ServerConfig::errorPage(int code) const
{
    std::map<int, std::string>::const_iterator it = error_pages.find(code);
    if (it == error_pages.end())
        return "";
    return it->second;
}
