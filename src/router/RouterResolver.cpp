#include "HttpRequest.hpp"
#include "Router.hpp"
#include "RouteResult.hpp"
#include "Method.hpp"
#include "Status.hpp"
#include <cstddef>
#include <string>
#include <sys/stat.h>

bool Router::isMethodAllowed(RouteResult& result, Method method)
{
    if (result.location == NULL)
        return true;

    if (result.location->allowed_methods.empty())
        return true;

    for (std::size_t i = 0; i < result.location->allowed_methods.size(); i++) {
        if (result.location->allowed_methods[i] == method)
            return true;
    }
    return false;
}

std::string Router::getExtension(const std::string& path) { // utils

    std::size_t slashPos = path.rfind('/');
    std::size_t dotPos = path.rfind('.');

    if (dotPos == std::string::npos || dotPos < slashPos) { // dotPos < slashPos + 1 for case path = blabla/.hiddenFile
        return "";
    }
    return path.substr(dotPos + 1);
}

bool Router::isCgiRequest(RouteResult& result, const std::string& path) {
    if (result.location == NULL || result.location->cgi.empty())
        return false;

    std::string ext = getExtension(path);
    if (ext.empty())
        return false;
    ext += ".";

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
        // result.action = ROUTE_ERROR;
        // result.statusCode = status::NOT_FOUND;
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

bool Router::checkPermission(const std::string &path, PermissionTarget target)
{
    const char* p = path.c_str();

    if (target == PERM_READ_FILE)
        return canRead(p);
    if (target == PERM_WRITE_FILE)
        return canWrite(p);
    if (target == PERM_EXECUTE_FILE)
        return canExecute(p);
    if (target == PERM_DELETE_FILE)
        return canDelete(getParentDirectory(path).c_str());

    return true;
}

bool Router::canRead(const char *p)
{
    if (access(p, R_OK) != 0)
        return false;
    return true;
}

bool Router::canWrite(const char *p)
{
    if (access(p, W_OK) != 0)
        return false;
    return true;
}

bool Router::canExecute(const char *p)
{
    if (access(p, X_OK) != 0)
        return false;
    return true;
}

bool Router::canDelete(const char *p)
{
    if (access(p, W_OK | X_OK) != 0)
        return false;
    return true;
}

std::string Router::getParentDirectory(const std::string &path) // not sure if full correct!
{
    std::size_t pos = path.rfind('/');
    if (pos != std::string::npos)
        return "/";
    return path.substr(0, pos);
}

PermissionTarget Router::permissionFromRequest(const RouteResult& route, Method method)
{
    if (route.action == ROUTE_CGI)  return PERM_EXECUTE_FILE;
    if (method == method::GET)      return PERM_READ_FILE;
    if (method == method::POST)     return PERM_WRITE_FILE;
    if (method == method::DELETE)   return PERM_DELETE_FILE;

    return PERM_READ_FILE;
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
