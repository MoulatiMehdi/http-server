#include "HttpRequest.hpp"
#include "Router.hpp"
#include "RouteResult.hpp"
#include "Method.hpp"
#include "Status.hpp"
#include <string>
#include <sys/stat.h>

bool Router::isMethodAllowed(RouteResult& result,
                             Method method)
{
    if (result.location == NULL)
        return true;

    if (result.location->allowed_methods.empty())
        return true;

    for (std::size_t i = 0; i < result.location->allowed_methods.size(); i++) {
        if (result.location->allowed_methods[i] == method)
            return true;
    }
    result.action = ROUTE_ERROR;
    result.statusCode = status::METHOD_NOT_ALLOWED;
    return false;
}

bool Router::isCgiRequest(RouteResult& result, const std::string& path) {
    if (result.location == NULL || result.location->cgi.empty())
        return false;

    std::size_t slashPos = path.rfind('/');
    std::size_t dotPos = path.rfind('.');

    if (dotPos == std::string::npos || dotPos < slashPos) { // dotPos < slashPos + 1 for case path = blabla/.hiddenFile
        return false;
    }

    std::string ext = path.substr(dotPos);

    if (result.location->cgi.find(ext) != result.location->cgi.end()) {
        result.action = ROUTE_CGI;
        result.statusCode = status::OK;
        return true;
    }

    return false;
}

bool Router::isUploadRequest(RouteResult &result, const HttpRequest& request) {
    if (result.location == NULL)
        return false;

    if (request.method() != method::POST)
        return false;

    if (result.location->upload_dir.empty()) { // can be empty!? also upload_enabled should be checked
        result.action = ROUTE_ERROR;
        result.statusCode = status::INTERNAL_SERVER_ERROR;
        return true;
    }
    result.action = ROUTE_UPLOAD;
    result.path = result.location->upload_dir;
    result.statusCode = status::OK;
    return true;
}

//  TODO: check syntax later
//  re check

bool Router::pathExists(RouteResult& result) {
    struct stat st;

    if (stat(result.path.c_str(), &st) != 0) {
        result.action = ROUTE_ERROR;
        result.statusCode = status::NOT_FOUND;
        return true;
    }
    return false;
}

bool Router::isRegularFile(RouteResult& result) {
    struct stat st;
    if (stat(result.path.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
        result.action = ROUTE_STATIC_FILE;
        result.statusCode = status::OK;
        return true;
    }
    return false;
}

bool Router::isDirectory(RouteResult& result) {
    struct stat st;
    if (stat(result.path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        result.action = ROUTE_DIRECTORY_LISTING;
        result.statusCode = status::OK;
        return true;
    }
    return false;
}

// -------------------------- 

bool Router::readPermission(RouteResult& result, const char *p)
{
    if (access(p, R_OK) != 0) {
        result.action = ROUTE_ERROR;
        result.statusCode = status::FORBIDDEN;
        return false;
    }
    return true;
}

bool Router::writePermission(RouteResult& result, const char *p)
{
    if (access(p, W_OK) != 0) {
        result.action = ROUTE_ERROR;
        result.statusCode = status::FORBIDDEN;
        return false;
    }
    return true;
}

bool Router::executePermission(RouteResult& result, const char *p)
{
    if (access(p, X_OK) != 0) {
        result.action = ROUTE_ERROR;
        result.statusCode = status::FORBIDDEN;
        return false;
    }
    return true;
}


bool Router::deletePermission(RouteResult& result) // check r&w
{
    std::string dir = result.path;
    std::size_t pos = dir.rfind('/');
    if (pos != std::string::npos)
        dir = (pos == 0) ? "/" : dir.substr(0, pos);

    if (access(dir.c_str(), W_OK) != 0) {
        result.action = ROUTE_ERROR;
        result.statusCode = status::FORBIDDEN;
        return false;
    }
    return true;
}


bool Router::checkPermission(RouteResult& result, Method method)
{
    const char* p = result.path.c_str();

    if (result.action == ROUTE_CGI)
        return executePermission(result, p);
    if (method == method::GET)
        return readPermission(result, p);
    if (method == method::POST) // not needed ?
        return writePermission(result ,p);
    if (method == method::DELETE)
        deletePermission(result);

    return true;
}

// debugging

#include <iostream>
#include "Status.hpp"

static std::string actionToString(RouteAction action) {
    switch (action) {
        case ROUTE_STATIC_FILE:       return "STATIC_FILE";
        case ROUTE_CGI:               return "CGI";
        case ROUTE_DIRECTORY_LISTING: return "DIRECTORY_LISTING";
        case ROUTE_UPLOAD:            return "UPLOAD";
        case ROUTE_ERROR:             return "ERROR";
        default:                      return "UNKNOWN";
    }
}

void Router::printRouteResult(const RouteResult& r) {
    std::cout << "\n=== RouteResult ===\n";

    std::cout << "Server:   "
              << (r.server ? "set" : "NULL") << "\n";

    std::cout << "Location: "
              << (r.location ? r.location->path : "NULL") << "\n";

    std::cout << "Action:   "
              << actionToString(r.action) << "\n";

    std::cout << "Path:     "
              << r.path << "\n";

    std::cout << "Status:   "
              << static_cast<int>(r.statusCode)   // 👈 numeric code
              << " "
              << phrase_reason(r.statusCode)      // 👈 text (e.g. "Not Found")
              << "\n";

    std::cout << "===================\n";
}
