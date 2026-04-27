#ifndef UPLOAD_HANDLER_HPP
#define UPLOAD_HANDLER_HPP

#include "HttpRequest.hpp"
#include "RouteResult.hpp"
#include <string>

class UploadHandler {
public:
    static bool handle(const HttpRequest& request, const RouteResult& route);

private:
    static std::string getBoundary(const HttpRequest& req);
    static std::string getFilename(const std::string& partHeader);
    static bool writeFile(const std::string &path, const std::string& content);
};

#endif
