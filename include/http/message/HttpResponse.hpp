#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "HttpMessage.hpp"
#include "HttpResponseParser.hpp"
#include <string>

class HttpResponseParser;

class HttpResponse : public HttpMessage
{
  private:
    HttpResponseParser m_parser;

  public:
    HttpResponse();
    ~HttpResponse();

    size_t              gcount() const;
    void                parse(const char *c_str, size_t len);
    HttpResponseParser &parser();

    std::string to_string() const;
    std::string serve_page();
    std::string
    serve_directory(const std::string &root, std::string path);
};

#endif
