#include "Router.hpp"
#include "RouteResult.hpp"
// #include "ConfigParser.hpp"

bool Router::isMethodAllowed(RouteResult& result,
                             const std::string& method)
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
    result.statusCode = 405;
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
        result.statusCode = 200;
        return true;
    }

    return false;
}

//  TODO: check syntax later

bool Router::pathExists(RouteResult& result) {
    struct stat st;

    if (stat(result.path.c_str(), &st) == 0) {
        result.action = ROUTE_ERROR;
        result.statusCode = 404;
        return true;
    }
    return false;
}

bool Router::isRegularFile(RouteResult& result) {
    struct stat st;
    if (stat(result.path.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
        result.action = ROUTE_STATIC_FILE;
        result.statusCode = 200;
        return true;
    }
    return false;
}

bool Router::isDirectory(RouteResult& result) {
    struct stat st;
    if (stat(result.path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        result.action = ROUTE_DIRECTORY_LISTING;
        result.statusCode = 200;
        return true;
    }
    return false;
}

// --------------------------

bool Router::readPermition(RouteResult& result, char *p)
{
    if (access(p, R_OK) != 0) {
        result.action = ROUTE_ERROR;
        result.statusCode = 403;
        return false;
    }
    return true;
}

bool Router::writePermition(RouteResult& result, char *p)
{
    if (access(p, W_OK) != 0) {
        result.action = ROUTE_ERROR;
        result.statusCode = 403;
        return false;
    }
    return true;
}

bool Router::executePermition(RouteResult& result, char *p)
{
    if (access(p, X_OK) != 0) {
        result.action = ROUTE_ERROR;
        result.statusCode = 403;
        return false;
    }
    return true;
}


bool Router::deletePermition(RouteResult& result)
{
    std::string dir = result.path;
    std::size_t pos = dir.rfind('/');
    if (pos != std::string::npos)
        dir = (pos == 0) ? "/" : dir.substr(0, pos);

    if (access(dir.c_str(), W_OK) != 0) {
        result.action = ROUTE_ERROR;
        result.statusCode = 403;
        return false;
    }
    return true;
}


bool Router::checkPermission(RouteResult& result, const std::string& method)
{
    const char* p = result.path.c_str();

    if (method == "GET")
        return readPermition(result, p);

    if (method == "POST")
        return writePermition(result ,p);

    if (result.action == ROUTE_CGI)
        return executePermition(result, p);

    if (method == "DELETE")
        deletePermition(result);

    return true;
}
