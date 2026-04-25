#ifndef UPLOAD_HANDLER_HPP
#define UPLOAD_HANDLER_HPP

#include "HttpRequest.hpp"
#include "RouteResult.hpp"

class UploadHandler {
public:
    static HttpResponse handle(const HttpRequest& request, const RouteResult& route);

    private:
};

#endif
