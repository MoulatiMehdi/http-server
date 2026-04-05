#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP
#include "Status.hpp"
#include <ostream>

#include "HttpMessage.hpp"
#include <map>
#include <string>

class HttpRequest : public HttpMessage
{
  public:
    typedef std::map<const std::string, std::string> Headers;

  private:
    size_t      m_content_length;
    std::string m_target; // uri
    Headers     m_headers;
    Status      m_status;
    std::string m_body;

  public:
    HttpRequest();
    HttpRequest(const HttpRequest &);
    ~HttpRequest();

    HttpRequest &operator=(const HttpRequest &);

    void set_target(const std::string &uri);

    const Headers &headers() const;
    Headers       &headers();

    void setHeader(std::string &name, std::string &value);
    bool good() const;

    const std::string &target() const;
    std::string       &target();
    const std::string &body() const;
    std::string       &body();
};

std::ostream &operator<<(std::ostream &os, const HttpRequest &request);
#endif
