#include "Router.hpp"
#include "Config.hpp"
#include "RouteResult.hpp"
#include "HttpRequest.hpp"
#include "Status.hpp"
#include <map>
#include <string>
#include <iostream>

RouteResult Router::resolve(const ServerConfig& server, const HttpRequest& request)
{
    RouteResult result;

    if (!request.good())
        return errorPage(request.status(), server.error_pages);

    result.server = &server;
    std::size_t pathPos = request.uri().find('?');
    std::string path = request.uri().substr(0, pathPos); // use funciton

    result.location = matchLocation(server, path); 
    if (result.location)
        std::cout << "Matched location: " << result.location->path << "\n";
    if (!isMethodAllowed(result, request.method()))
        return errorPage(status::METHOD_NOT_ALLOWED, server.error_pages);

    result.path = buildTargetPath(server, result.location, path);

    if (isRedirect(result))            return result;
    if (isCgiRequest(result, path))       return result; 
    if (isUploadRequest(result, request)) return result; // path!?
    if (!pathExists(result.path))
        return errorPage(status::NOT_FOUND, server.error_pages);
    if (!checkPermission(result.path, permissionFromRequest(result, request.method())))
        return errorPage(status::FORBIDDEN, server.error_pages);
    
    
    if (handleRegularFile(result))            return result;
    if (isDirectory(result, path) && isIndexed(result))
        return result;
    else if (result.location && result.location->autoindex == false)
        return errorPage(status::FORBIDDEN, server.error_pages);

    
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

bool Router::pathMatchesLocation(const std::string& requestPath, const std::string& locationPath) {
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
    std::string suffix = reqPath.substr(locPath.size());
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

    if (location != NULL) {
        if (!location->root.empty())
            root = location->root;
        suffix = extractSuffix(location->path, requestPath);
    }
    else
        suffix = extractSuffix("", requestPath);

    if (!root.empty() && root[root.size() - 1] != '/')
        root += "/";
    return root + suffix;
}
