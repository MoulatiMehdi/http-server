#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "HttpMessage.hpp"

class HttpResponse : public HttpMessage
{
  public:
    HttpResponse();
    ~HttpResponse();

    std::string to_string() const;
};

#endif
