#ifndef ROUTE_RESULT_HPP
#define ROUTE_RESULT_HPP
#ifndef   ROUTERESULT_HPP
# define  ROUTERESULT_HPP

# include <string>
# include "ConfigParser.hpp"

enum RouteAction {
    ROUTE_STATIC_FILE,
    ROUTE_CGI,
    ROUTE_DIRECTORY_LISTING,
    ROUTE_ERROR
};

struct RouteResult {
    const ServerConfig*   server;
    const LocationConfig* location;

    RouteAction           action;
    std::string           path;
    int                   statusCode;

    RouteResult()
        : server(NULL),
          location(NULL),
          action(ROUTE_ERROR),
          path(""),
          statusCode(404) {}
};

#endif
#endif
