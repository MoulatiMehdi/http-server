#ifndef HTTP_REQUEST_PARSER_HPP
#define HTTP_REQUEST_PARSER_HPP

#include "Buffer.hpp"
#include "HttpParserState.hpp"
#include "HttpResponse.hpp"
#include "RouteResult.hpp"
#include <cstddef>
#include <string>

class HttpRequest;

class HttpRequestParser : public HttpParserState
{
  private:
    typedef unsigned int (HttpRequestParser::*Handler)(u_char);
    HttpRequest &m_request;

    void process_error();
    void process_headers();
    void process_content_length();
    void process_content_type();

    void parse_body(Buffer &buffer);
    void parse_request_line(Buffer &buff);
    void parse_headers(Buffer &buff);

    void parse_body_by_length(Buffer &buffer);
    void parse_body_by_chunk(Buffer &buffer);
    void parse_multipart(Buffer &buffer);
    void parse_upload_body(Buffer &buffer);

    std::string  m_filename;
    std::string  m_boundary;
    std::string  m_content_type;
    size_t       m_index;
    int          fd;
    HttpResponse m_response;

  public:
    RouteResult route;
    HttpRequestParser(HttpRequest &request);
    ~HttpRequestParser();

    void parse(const char *c_str, size_t len);
};

#endif
