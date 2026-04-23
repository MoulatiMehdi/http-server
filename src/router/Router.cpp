#include "Router.hpp"
#include "Config.hpp"
#include "RouteResult.hpp"
#include "HttpRequest.hpp"
#include "helper.hpp"
#include <iostream>
#include <ostream>

RouteResult Router::resolve(const ServerConfig& server, const HttpRequest& request)
{
    RouteResult result;

    result.server = &server;
    std::size_t pathPos = request.uri().find('?');
    std::string path = request.uri().substr(0, pathPos);
    
    std::cout << "method = " << to_string(request.method()) << std::endl;
    // std::cout << "$$$$$  requestPath = " << path << std::endl;

    result.location = matchLocation(server, path); 

    if (!isMethodAllowed(result, request.method()))
        return result;

    result.path = buildTargetPath(server, result.location, path);

    if (isCgiRequest(result, path)) return result;
    if (pathExists(result))        return result;
    if (!checkPermission(result, request.method()))   return result;
    if (isRegularFile(result))      return result;
    if (isDirectory(result))        return result;

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

    // std::cout << "reqPath = " << reqPath << std::endl;
    // std::cout << "suffix = " << suffix << std::endl;
    if (!suffix.empty() && suffix[0] == '/')
        suffix = suffix.substr(1);
    // std::cout << "suffix = " << suffix << std::endl;
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
        // std::cout << "root = " << location->root << std::endl;
        // std::cout << "root = " << root << std::endl;
        // if ()
        suffix = extractSuffix(location->path, requestPath);
    }
    else
        suffix = extractSuffix("", requestPath);

    if (!root.empty() && root[root.size() - 1] != '/')
        root += "/";
    return root + suffix;
}
