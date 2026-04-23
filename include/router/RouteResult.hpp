#ifndef ROUTE_RESULT_HPP
#define ROUTE_RESULT_HPP
#include "Status.hpp"
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
    status::Status                   statusCode;

    RouteResult()
        : server(NULL),
          location(NULL),
          action(ROUTE_ERROR),
          path(""),
          statusCode(status::NOT_FOUND) {}
};

#endif
#endif
