#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP
#include <ostream>

#include "HttpMessage.hpp"
#include "Method.hpp"
#include <string>

class HttpRequest : public HttpMessage
{
  private:
    std::string m_uri; // uri
    Method      m_method;
    bool        m_complete;

  public:
    HttpRequest();
    // HttpRequest(const HttpRequest &);
    // HttpRequest &operator=(const HttpRequest &);
    ~HttpRequest();


    bool good() const;
    bool complete() const;

    Method             method() const;
    const Headers     &headers() const;
    const std::string &uri() const;
    void               clear();

    std::string &uri();
    Headers     &headers();

    void setMethod(std::string &method);
    void setMethod(Method method);
    void setComplete(bool val);
    void setUri(const std::string &uri);
};

std::ostream &operator<<(std::ostream &os, const HttpRequest &request);
#endif
