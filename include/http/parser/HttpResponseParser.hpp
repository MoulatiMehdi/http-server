#ifndef HTTP_RESPONSE_PARSER_HPP
#define HTTP_RESPONSE_PARSER_HPP

#include "HttpParserState.hpp"
#include <cstddef>

class HttpResponse;

class HttpResponseParser : public HttpParserState
{
  private:
    typedef unsigned int (HttpResponseParser::*Handler)(u_char);
    HttpResponse &request;
    size_t        m_size;

    void process_error();
    void process_remove_headers();
    void process_headers();
    void process_content_length();
    void process_status();
    void process_header_line();
    void parse_headers(Buffer &buff);

  public:
    HttpResponseParser(HttpResponse &request);
    ~HttpResponseParser();

    size_t gcount() const;
    void   parse(const char *c_str, size_t len);
};

#endif
