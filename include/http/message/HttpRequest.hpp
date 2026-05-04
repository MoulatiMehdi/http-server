#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "Config.hpp"
#include "HttpMessage.hpp"
#include "HttpRequestParser.hpp"
#include "Method.hpp"
#include <string>

class HttpRequest : public HttpMessage
{
  private:
    std::string       m_uri; // uri
    Method            m_method;
    HttpRequestParser m_parser;

  public:
    const ServerConfig &config;
    HttpRequest(const ServerConfig &config);
    ~HttpRequest();

    using HttpMessage::good;

    Method             method() const;
    const std::string &uri() const;
    std::string        to_string() const;

    std::string &uri();

    void setMethod(std::string &method);
    void setMethod(Method method);
    void setUri(const std::string &uri);
    using HttpMessage::extract_key;

    HttpRequestParser &parser();
    void               parse(const char *c_str, size_t len);
};

#endif
