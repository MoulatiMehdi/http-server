#include "UploadHandler.hpp"
#include "HttpMessage.hpp"
#include "HttpRequest.hpp"
#include <cstddef>
#include <string>

bool UploadHandler::handle(const HttpRequest &request, const RouteResult &route)
{
    
}


std::string UploadHandler::getBoundary(const HttpRequest& req) {
    HttpMessage::const_iterator it = req.getHeader("Content-Type");
    if (it == req.headers().end())
        return "";
    std::string contentType = it->second;

    std::string key = "boundary=";
    std::size_t pos = contentType.find(key);

    if (pos == std::string::npos)
        return "";
    
    return contentType.substr(pos + key.size());
}
