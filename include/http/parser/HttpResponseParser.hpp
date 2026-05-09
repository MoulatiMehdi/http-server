#ifndef HTTP_RESPONSE_PARSER_HPP
#define HTTP_RESPONSE_PARSER_HPP

#include "Buffer.hpp"
#include "HttpParserState.hpp"
#include <cstddef>

class HttpResponse;

class HttpResponseParser : public HttpParserState
{
  private:
    typedef unsigned int (HttpResponseParser::*Handler)(u_char);
    HttpResponse &m_response;
    size_t        m_size;

  public:
    size_t m_code;
    void   process_error();
    void   process_headers();
    void   process_content_length();
    void   process_status();

    void parse_headers(Buffer &buff);

    HttpResponseParser(HttpResponse &request);
    ~HttpResponseParser();

    void clear();

    size_t gcount() const;
    void   parse(const char *c_str, size_t len);
};

#endif
