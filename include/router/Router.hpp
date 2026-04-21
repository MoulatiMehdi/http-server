#ifndef   ROUTER_HPP
# define  ROUTER_HPP

# include <string>
# include "Config.hpp"
# include "HttpRequest.hpp" // check name
# include "RouteResult.hpp"
    

class Router {
public:
    static RouteResult resolve(const ServerConfig& server, const HttpRequest& request);

private:
    Router();
    Router(const Router&);
    Router& operator=(const Router&);
    
    static const LocationConfig* matchLocation(const ServerConfig& server, const std::string& requestPath);
    
    static bool pathMatchesLocation(const std::string& requestPath, const std::string& locationPath);
    
    static bool isMethodAllowed(RouteResult &result, const std::string& method);

    static bool isCgiRequest(RouteResult &result, const std::string& path);

    static std::string buildTargetPath(const ServerConfig& server, const LocationConfig* location,
                                       const std::string& requestPath);

    static bool pathExists(RouteResult &result);
    static bool isRegularFile(RouteResult &result);
    static bool isDirectory(RouteResult &result);

    std::string extractSuffix(const std::string& locPath, const std::string& reqPath);
    
    bool readPermition(RouteResult& result, char *p);
    bool writePermition(RouteResult& result, char *p);
    bool executePermition(RouteResult& result, char *p);
    bool deletePermition(RouteResult& result);
};

#endif
