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

  private:
    std::string m_target; // uri
    Headers     m_headers;
    bool        m_complete;

  public:
    HttpRequest();
    HttpRequest(const HttpRequest &);
    ~HttpRequest();

    HttpRequest &operator=(const HttpRequest &);

    HttpRequest::Headers::const_iterator
    get(const std::string &header_name) const;

    void setComplete(bool val);
    void set(const std::string &name, const std::string &value);

    bool good() const;
    bool incomplete() const;

    const Headers &headers() const;
    Headers       &headers();

    void               setTarget(const std::string &uri);
    const std::string &target() const;
    std::string       &target();
};

std::ostream &operator<<(std::ostream &os, const HttpRequest &request);
#endif
