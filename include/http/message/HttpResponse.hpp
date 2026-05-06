#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "HttpMessage.hpp"
#include "HttpResponseParser.hpp"
#include "Status.hpp"
#include <set>
#include <string>

class HttpResponseParser;

class HttpResponse : public HttpMessage
{
    typedef std::set<std::string, HttpMessage::iless> Set;

  private:
    Set                allowed_headers;
    HttpResponseParser m_parser;

  public:
    HttpResponse();
    HttpResponse(Status status);
    ~HttpResponse();

    size_t              gcount() const;
    void                parse(const char *c_str, size_t len);
    HttpResponseParser &parser();
    void                clear();

    using HttpMessage::extract_key;
    void        add_server_headers();
    std::string to_string();
    std::string serve_page();
    std::string serve_directory(const std::string &root, std::string path);
};

#endif
