#ifndef   ROUTER_HPP
# define  ROUTER_HPP

# include <string>
# include "Config.hpp"
# include "HttpRequest.hpp" // check name
# include "RouteResult.hpp"
# include "Method.hpp"

class Router {
public:
    static RouteResult resolve(const ServerConfig& server, const HttpRequest& request);

private:
    Router();
    Router(const Router&);
    Router& operator=(const Router&);
    
    static const LocationConfig* matchLocation(const ServerConfig& server, const std::string& requestPath);
    
    static bool pathMatchesLocation(const std::string& requestPath, const std::string& locationPath);
    
    static bool isMethodAllowed(RouteResult &result, Method method);

    static bool isCgiRequest(RouteResult &result, const std::string& path);

    static std::string buildTargetPath(const ServerConfig& server, const LocationConfig* location,
                                       const std::string& requestPath);

    static bool pathExists(RouteResult &result);
    static bool isRegularFile(RouteResult &result);
    static bool isDirectory(RouteResult &result);
    static bool checkPermission(RouteResult &result, Method method);

    static std::string extractSuffix(const std::string& locPath, const std::string& reqPath);
    
    static bool readPermission(RouteResult& result, const char *p);
    static bool writePermission(RouteResult& result, const char *p);
    static bool executePermission(RouteResult& result, const char *p);
    static bool deletePermission(RouteResult& result);
};

#endif
