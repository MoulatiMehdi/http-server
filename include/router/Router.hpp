#ifndef   ROUTER_HPP
# define  ROUTER_HPP

#include <map>
# include <string>
# include "Config.hpp"
# include "HttpRequest.hpp" // check name
# include "RouteResult.hpp"
# include "Method.hpp"

enum PermissionTarget {
    PERM_READ_FILE,
    PERM_WRITE_FILE,
    PERM_EXECUTE_FILE,
    PERM_DELETE_FILE
};

class Router {
public:
    static RouteResult resolve(const ServerConfig& server, const HttpRequest& request);

    // debugging
    static void printRouteResult(const RouteResult& r);
    static std::string getParentDirectory(const std::string &path);

private:
    Router();
    Router(const Router&);
    Router& operator=(const Router&);
    
    // RouteResult &result;

    static std::string getExtension(const std::string& path);

    static const LocationConfig* matchLocation(const ServerConfig& server, const std::string& requestPath);
    
    static bool pathMatchesLocation(const std::string& requestPath, const std::string& locationPath);
    
    static bool isRedirect(RouteResult& result);
    static bool isMethodAllowed(RouteResult &result, Method method);
    static bool isCgiRequest(RouteResult &result, const std::string& path);
    static bool isUploadRequest(RouteResult &result, const HttpRequest& request);
    static bool isDeleteMethod(RouteResult& result, Method method);

    static std::string buildTargetPath(const ServerConfig& server, const LocationConfig* location,
                                       const std::string& requestPath);

    static std::string extractSuffix(const std::string& locPath, const std::string& reqPath);
    
    static bool pathExists(std::string &path);
    static bool handleRegularFile(RouteResult &result); // ++
    static bool isDirectory(RouteResult &result, std::string& requestPath);
    static bool isIndexed(RouteResult& result);

    static void indexResult(RouteResult& result, std::string& path);

    static bool checkPermission(const std::string &path, PermissionTarget target);
    
    static bool canRead(const char *p);
    static bool canWrite(const char *p);
    static bool canExecute(const char *p);
    static bool canDelete(const char *p);
    
    static bool isFile(const std::string& path);

    static bool findIndexFile(RouteResult& result, IndexTable& index,
                               std::string& root);

    static PermissionTarget permissionFromRequest(const RouteResult& route, Method method);
 
    static bool putFileOnDir(const std::string &src, const std::string &dest);   


    static bool deleteFile(const std::string& path, status::Status& code);
    // Error
    static RouteResult errorPage(Status status, std::map<int, std::string> error_pages);
};

#endif
