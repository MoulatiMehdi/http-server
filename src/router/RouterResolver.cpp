#include "Config.hpp"
#include "HttpRequest.hpp"
#include "Logger.hpp"
#include "Router.hpp"
#include "RouteResult.hpp"
#include "Method.hpp"
#include "Status.hpp"
#include "helper.hpp"
#include <cstddef>
#include <string>
#include <sys/stat.h>
#include <cstdio>
#include <vector>
#include <iostream>

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

bool Router::isRedirect(RouteResult& result)
{
    if (!result.location)
        return false;

    if (result.location->redirect_code == 0)
        return false;

    if (result.location->redirect_url.empty()) {
        result.action = ROUTE_ERROR;
        result.statusCode = status::INTERNAL_SERVER_ERROR;
        return true;
    }

    result.action = ROUTE_REDIRECT;
    result.statusCode =
        static_cast<status::Status>(result.location->redirect_code);
    result.path = result.location->redirect_url;

    return true;
}

// /hello/test.py/testof -> /hello/test.py & /testof

// std::vector<std::string> getAllExtention(const std::map<std::string, std::string> cgi) {
//     std::map<std::string, std::string>::iterator it;
//     std::vector<std::string> allExt;
//
//     for (it = cgi.begin(); it != cgi.end(); ++it) {
//         allExt.push_back(it->first);
//     }
//     return allExt;
// }

std::size_t getExtentionPos(const std::string &path, const std::string &ext)
{
    std::size_t i = path.find(ext + "/");
    
    if (i != std::string::npos)
        return i;
    // std::cout << "last of = " << path.substr(path.size()- ext.size()) << "\n";
    i = path.rfind(ext);
    if (std::string::npos == i)
        return i;
    if (path.size() == i + ext.size())
        return i;
    return std::string::npos;
}

string_map::const_iterator getScriptNamePos(RouteResult& result, const std::string& path, std::size_t& min)
{
    string_map::const_iterator it;
    string_map::const_iterator itBest;
    const string_map& cgi = result.location->cgi;
    min = std::string::npos;

    for (it = cgi.begin(); it != cgi.end(); ++it) {
        std::size_t i = getExtentionPos(path, it->first);
        if (min > i) {
            min = i;
            itBest = it;
        }
    }
    if (min == std::string::npos)
        return result.location->cgi.end();
    min += itBest->first.size();
    return itBest;
}


bool Router::isCgiRequest(RouteResult& result, const std::string& reqPath)
{
    if (result.location == NULL || result.location->cgi.empty())
        return false;

    std::size_t minPos;
    string_map::const_iterator itBest = getScriptNamePos(result, reqPath, minPos);
    if (itBest == result.location->cgi.end())
        return false;
    result.scriptName = reqPath.substr(0, minPos);
    result.cmd = itBest->second;
    if (minPos == result.scriptName.size())
        result.pathInfo = reqPath.substr(minPos);
    if (result.pathInfo.empty())
        result.pathInfo = "/";
    getScriptNamePos(result, result.path, minPos);
    if (minPos == std::string::npos) {
        Logger::info("CGI (Router): Abnormal case !!!!\n");
        return true;
    }
    result.path = result.path.substr(0, minPos);
    if (!isFile(result.path)) {
        result.action = ROUTE_ERROR;
        result.statusCode = status::NOT_FOUND;
        return true; 
    }
    result.action = ROUTE_CGI;
    result.statusCode = status::OK;
    return true;
}
//
// bool Router::isCgiRequest(RouteResult& result, const std::string& path) {
//     if (result.location == NULL || result.location->cgi.empty())
//         return false;
//
//     std::string ext = getExtension(path);
//     if (ext.empty())
//         return false;
//     ext = "." + ext;
//     std::map<std::string, std::string>::const_iterator it = result.location->cgi.find(ext);
//     if (it != result.location->cgi.end()) {
//         if (!isFile(result.path.c_str())) {
//             result.action = ROUTE_ERROR;
//             result.statusCode = status::NOT_FOUND;
//             return true; 
//         }
//         // if (!canExecute(result.path.c_str()) || !canRead(result.path.c_str())) {
//         //     result.action = ROUTE_ERROR;
//         //     result.statusCode = status::FORBIDDEN;
//         //     return true;
//         // }
//         result.action = ROUTE_CGI;
//         result.statusCode = status::OK;
//         result.cmd = it->second;
//         return true;
//     }
//
//     return false;
// }

bool Router::isUploadRequest(RouteResult &result, const HttpRequest& request) {
    if (result.location == NULL)
        return false;

    if (request.method() != method::POST)
        return false;

    std::string root = result.location->root.empty() ?
                       result.server->root : result.location->root;
    result.action = ROUTE_UPLOAD;
    result.path = root; 
    result.statusCode = status::CREATED;
    return true;
}

bool Router::pathExists(std::string& path) {
    struct stat st;

    if (stat(path.c_str(), &st) == 0) {
        return true;
    }
    return false;
}

bool Router::handleRegularFile(RouteResult& result) {
    // if (result.path[result.path.size() - 1] != '/' && isFile(result.path)) {
    if (isFile(result.path)) {
        result.action = ROUTE_STATIC_FILE;
        result.statusCode = status::OK;
        result.type = ServerConfig::mimetype.getContentType(getExtension(result.path)); // I think must be for regulare files only
        return true;
    }
    return false;
}


bool Router::isFile(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode))
        return true;
    return false;
}

void Router::indexResult(RouteResult& result, std::string& path) {
    result.action = ROUTE_STATIC_FILE;
    result.path = path;
    result.statusCode = status::OK;
    result.type = ServerConfig::mimetype.getContentType(getExtension(result.path)); // I think must be for regulare files only
}

bool Router::findIndexFile(RouteResult& result, IndexTable& index, std::string& root)
{
    for (std::size_t i = 0; i < index.size(); i++) {
        std::string path = root + index[i];
        if (isFile(path) && canRead(path.c_str()))
            return indexResult(result, path), true;
    }
    return false;
}

bool Router::isIndexed(RouteResult& result)
{
    if (result.action == ROUTE_REDIRECT)
        return true;
    // if (result.location && !pathMatchesLocation(result.path, result.location->path))
    //     return true;
    if ((result.location &&
        ((result.location->index.empty()) && result.location->autoindex == true)))
        return false;
    if (result.location == NULL && result.server->index.empty())
        return false;

    IndexTable  index = (result.location && !result.location->index.empty()) ?
                         result.location->index : result.server->index;
    // std::string root  = (result.location && !result.location->root.empty())  ?
    //                      result.location->root  : result.server->root;
    if (findIndexFile(result, index, result.path))
        return true;
    if (!result.location->index.empty()) {
        result = errorPage(status::NOT_FOUND, result.server->error_pages);
        return true; 
    }
    return false;
}

bool Router::isDirectory(RouteResult& result, std::string& requestPath) {
    struct stat st;
    if (stat(result.path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        if (result.path[result.path.size() - 1] != '/') {
            result.action = ROUTE_REDIRECT;
            result.statusCode = status::MOVED_PERMANENTLY;
            result.path = requestPath + "/";
            return true;
        }
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
    if (pos == std::string::npos)
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
        case ROUTE_REDIRECT:          return "REDIRECT";
        case ROUTE_DELETE:            return "DELETE";
        default:                      return "UNKNOWN";
    }
}

void Router::printRouteResult(const RouteResult& r) {
    std::cout << "\n=== RouteResult ===\n";

    std::cout << "Server    : "
              << (r.server ? "set ✔" : "NULL") << "\n";

    std::cout << "Location  : "
              << (r.location ? r.location->path : "NULL") << "\n";

    std::cout << "Action    : "
              << actionToString(r.action) << "\n";
    if (r.action == ROUTE_CGI)
        std::cout << "ext       : " << r.cmd << "\n";
    if (!r.path.empty())
        std::cout << "Path      : " << r.path << "\n";
    if (!r.type.empty())
        std::cout << "Type      : " << r.type << "\n";
    if (!r.scriptName.empty()) 
        std::cout << "scriptName: " << r.scriptName << "\n";
    if (!r.pathInfo.empty())
        std::cout << "pathInfo  : " << r.pathInfo << "\n";

    std::cout << "Status    : "
              << static_cast<int>(r.statusCode)   // 👈 numeric code
              << " "
              << phrase_reason(r.statusCode)      // 👈 text (e.g. "Not Found")
              << "\n";

    std::cout << "===================\n";
}
