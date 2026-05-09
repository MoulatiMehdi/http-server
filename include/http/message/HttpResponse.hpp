#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "HttpMessage.hpp"
#include "HttpResponseParser.hpp"
#include "Status.hpp"
#include <string>

class HttpResponse : public HttpMessage
{
  private:
    HttpResponseParser m_parser;

  public:
    HttpResponse();
    HttpResponse(Status status);
    ~HttpResponse();

    using HttpMessage::extract_key;

    size_t gcount() const;
    HttpResponseParser &parser();

    void parse(const char *c_str, size_t len);
    void add_server_headers();
    void clear();

    std::string to_string();
    std::string serve_page();
    std::string serve_directory(const std::string &root, std::string path);
};

#endif
