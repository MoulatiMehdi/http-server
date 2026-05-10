#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "Config.hpp"
#include "HttpMessage.hpp"
#include "HttpRequestParser.hpp"
#include "Method.hpp"
#include "Uri.hpp"
#include <cstddef>
#include <string>

class HttpRequest : public HttpMessage
{
  private:
    Uri               m_uri; // uri
    Method            m_method;
    HttpRequestParser m_parser;
    size_t            m_max_body_size;

  public:
    const ServerConfig &m_server_config;
    HttpRequest(const ServerConfig &config);
    ~HttpRequest();

    using HttpMessage::good;

    Method      method() const;
    std::string to_string() const;

    Uri       &uri();
    const Uri &uri() const;
    size_t     maxBodySize() const;

    void setMaxBodySize(size_t size);
    void setMethod(std::string &method);
    void setMethod(Method method);
    void setUri(const std::string &uri);

    using HttpMessage::extract_key;

    HttpRequestParser &parser();
    void               parse(const char *c_str, size_t len);
};

#endif
