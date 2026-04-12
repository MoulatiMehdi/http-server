#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP
#include <ostream>

#include "HttpMessage.hpp"

class HttpResponse : public HttpMessage
{
  private:
    bool m_complete;

  public:
    HttpResponse();
    HttpResponse(const HttpResponse &);
    ~HttpResponse();

    HttpResponse &operator=(const HttpResponse &);

    bool good() const;
    bool complete() const;
    void setComplete(bool val);
};

std::ostream &operator<<(std::ostream &os, const HttpResponse &request);
#endif
