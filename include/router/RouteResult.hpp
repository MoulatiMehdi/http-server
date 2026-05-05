#ifndef ROUTE_RESULT_HPP
#define ROUTE_RESULT_HPP

# include <string>
# include "Config.hpp"
# include "Status.hpp"

struct ServerConfig;
struct LocationConfig;

enum RouteAction {
    ROUTE_STATIC_FILE,
    ROUTE_CGI,
    ROUTE_DIRECTORY_LISTING,
    ROUTE_UPLOAD,
    ROUTE_REDIRECT,
    ROUTE_DELETE,
    ROUTE_ERROR
};

struct RouteResult {
    const ServerConfig*   server;
    const LocationConfig* location;

    RouteAction           action;
    std::string           path;
    status::Status        statusCode;
    std::string           type;

    RouteResult()
        : server(NULL),
          location(NULL),
          action(ROUTE_ERROR),
          path(""),
          statusCode(status::NOT_FOUND),
          type("") {}
};

#endif
