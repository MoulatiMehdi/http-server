#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP
#include <ostream>

#include "HttpMessage.hpp"
#include <map>
#include <string>

class HttpRequest : public HttpMessage
{
    const std::string EMPTY = "";

  public:
    typedef std::map<const std::string, std::string> Headers;
    typedef HttpRequest::Headers::const_iterator     const_iterator;

  private:
    std::string m_uri; // uri
    Headers     m_headers;
    bool        m_complete;

  public:
    HttpRequest();
    HttpRequest(const HttpRequest &);
    ~HttpRequest();

    HttpRequest &operator=(const HttpRequest &);

    bool good() const;
    bool complete() const;

    const_iterator     getHeader(const std::string &name) const;
    const Headers     &headers() const;
    const std::string &uri() const;

    std::string &uri();
    Headers     &headers();

    void setComplete(bool val);
    void setTarget(const std::string &uri);
    void setHeader(const std::string &name, const std::string &value);
};

std::ostream &operator<<(std::ostream &os, const HttpRequest &request);
#endif
