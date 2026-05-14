#include "Router.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include "RouteResult.hpp"
#include "HttpRequest.hpp"
#include "Status.hpp"
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <cstdio>
#include <sys/stat.h>

RouteResult Router::resolve(const ServerConfig& server, const HttpRequest& request)
{
    RouteResult result;

    if (!request.good())
        return errorPage(request.status(), server.error_pages);

    result.server = &server;
    std::string reqPath = request.uri().path(); // use funciton
    
    // std::string tmpPath = path;
    // if (tmpPath[tmpPath.size() - 1] != '/')
    //     tmpPath += "/";

    result.location = matchLocation(server, reqPath); 
    if (result.location)
        std::cout << "matched location: " << result.location->path << "\n";
    if (!isMethodAllowed(result, request.method()))
        return errorPage(status::METHOD_NOT_ALLOWED, server.error_pages);

    result.path = buildTargetPath(server, result.location, reqPath);
	// std::cout << "path: "<< result.path << "\n";

    std::cout << "start validators..\n";
    if (isRedirect(result))
        return result;
    std::cout << "[]: isRedirect pass\n";
    if (isCgiRequest(result, reqPath))
        return result;
    std::cout << "[]: isCgiRequest pass\n";
    std::cout << "[]: pathExists checkin..\n";
    if (!pathExists(result.path))
        return errorPage(status::NOT_FOUND, server.error_pages);
    
    if (isUploadRequest(result, request))
        return result;
    std::cout << "[]: pathExists success\n";
    if (!checkPermission(result.path, permissionFromRequest(result, request.method())))
        return errorPage(status::FORBIDDEN, server.error_pages);
    std::cout << "[]: checkPermission success\n";
    if (isDeleteMethod(result, request.method()))
        return result;
    std::cout << "[]: isDeleteMethod pass\n";
    if (handleRegularFile(result))            return result;
    std::cout << "[]: handleRegularFile pass\n";
    if (isDirectory(result, reqPath) && isIndexed(result))
        return result;
    else if (result.location && result.location->autoindex == false)
        return errorPage(status::FORBIDDEN, server.error_pages);
    std::cout << "[]: isDirectory pass\n";
    return result;
}

RouteResult Router::errorPage(Status status, std::map<int, std::string> error_pages) // develop it.
{
    RouteResult result;

    result.action = ROUTE_ERROR;
    result.statusCode = status;
    result.path = "";
   
    std::map<int, std::string>::iterator it = error_pages.find(static_cast<int>(status));
    
    if (it == error_pages.end()) // use function from configParser (should be create first)
        return result;

    return result;
}

bool Router::deleteFile(const std::string& path, status::Status& code)
{
    struct stat st;

    if (stat(path.c_str(), &st) != 0) {
        Logger::error(path + ": " + "not found.");
        code = status::NOT_FOUND;
        return false;
    }

    if (S_ISDIR(st.st_mode) != 0) {
        Logger::error(path + ": " + "is a directory.");
        code = status::FORBIDDEN;
        return false;
    }

    if (std::remove(path.c_str()) != 0) {
        Logger::error(path + ": " + strerror(errno));
        code = status::INTERNAL_SERVER_ERROR;
        return false;
    }
    return true;
}

bool Router::isDeleteMethod(RouteResult& result, Method method) {
    if (method != method::DELETE)
        return false;

    status::Status code;
    if (!deleteFile(result.path, code)) {
        result.action = ROUTE_ERROR;
        result.statusCode = code;
        return true;
    }
    result.action = ROUTE_DELETE;
    result.statusCode = status::NO_CONTENT;
    return true;
}

const LocationConfig* Router::matchLocation(const ServerConfig& server,
                                            const std::string& requestPath)
{
    const LocationConfig* bestLocation = NULL;
    std::size_t bestLength = 0;

    for (std::size_t i = 0; i < server.locations.size(); i++) {
        const LocationConfig& loc = server.locations[i];
        
        if (!pathMatchesLocation(requestPath, loc.path))
            continue ;

        if (bestLocation == NULL || bestLength < loc.path.size()) {
            bestLocation = &loc;
            bestLength = loc.path.size();
        }
    }
    return bestLocation;
}

bool Router::pathMatchesLocation(const std::string& reqPath, const std::string& locationPath) {
    std::string requestPath = reqPath;
    if (requestPath[requestPath.size() - 1] != '/')
        requestPath += "/";
    std::size_t len = locationPath.length();
    if (len > requestPath.size())
        return false;

    std::string prefix = requestPath.substr(0, len);

    if (requestPath.compare(0, len, locationPath) != 0)
        return false;

    if (requestPath.size() == len)
        return true;

    if (locationPath[len - 1] == '/')
        return true;

    return requestPath[len] == '/';
}

std::string Router::extractSuffix(const std::string& locPath, const std::string& reqPath)
{
    std::size_t i = 0;
    if (!locPath.empty() && reqPath[reqPath.size() - 1] != '/')
        i = 1;
    std::string suffix = reqPath.substr(locPath.size() - i);
    if (!suffix.empty() && suffix[0] == '/')
        suffix = suffix.substr(1);
    return suffix;
}

std::string Router::buildTargetPath(const ServerConfig& server,
                                    const LocationConfig* location,
                                    const std::string& requestPath)
{
    std::string suffix = requestPath;
    std::string root = server.root;

    if (location != NULL && !location->root.empty()) {
        root = location->root;
        suffix = extractSuffix(location->path, requestPath);
    }
    else
        suffix = extractSuffix("/", requestPath);

    if (!root.empty() && root[root.size() - 1] != '/')
        root += "/";
    return root + suffix;
}
