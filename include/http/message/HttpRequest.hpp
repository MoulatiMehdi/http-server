#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "HttpMessage.hpp"
#include "Method.hpp"
#include <string>

class HttpRequest : public HttpMessage
{
  private:
    std::string m_uri; // uri
    Method      m_method;

  public:
    HttpRequest();
    ~HttpRequest();

    bool good() const;

    Method             method() const;
    const Headers     &headers() const;
    const std::string &uri() const;
    std::string        to_string() const;

    std::string &uri();
    Headers     &headers();

    void setMethod(std::string &method);
    void setMethod(Method method);
    void setUri(const std::string &uri);
};

#endif
