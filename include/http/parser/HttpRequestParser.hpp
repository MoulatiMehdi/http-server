#ifndef HTTP_REQUEST_PARSER_HPP
#define HTTP_REQUEST_PARSER_HPP

#include "HttpParserState.hpp"
#include <cstddef>

class HttpRequest;

class HttpRequestParser : public HttpParserState
{
  private:
    enum RequestLineResult
    {
        RES_ERROR,
        RES_CONTINUE,
        RES_METHOD_DONE,
        RES_URI_DONE,
        RES_VERSION_DONE,
        RES_REQUEST_LINE_DONE
    };

    typedef unsigned int (HttpRequestParser::*Handler)(u_char);
    HttpRequest &request;

    //  request
    unsigned int req_start(u_char ch);
    unsigned int req_method(u_char ch);
    unsigned int req_spaces_before_uri(u_char ch);
    unsigned int req_uri_after_slash(u_char ch);
    unsigned int req_check_uri(u_char ch);
    unsigned int req_uri(u_char ch);
    unsigned int req_http_09(u_char ch);
    unsigned int req_http_H(u_char ch);
    unsigned int req_http_HT(u_char ch);
    unsigned int req_http_HTT(u_char ch);
    unsigned int req_http_HTTP(u_char ch);
    unsigned int req_first_major_digit(u_char ch);
    unsigned int req_major_digit(u_char ch);
    unsigned int req_first_minor_digit(u_char ch);
    unsigned int req_minor_digit(u_char ch);
    unsigned int req_spaces_after_digit(u_char ch);
    unsigned int req_almost_done(u_char ch);

    void process_error();
    void parse_request_line(Buffer &buff);

  public:
    HttpRequestParser(HttpRequest &request);
    ~HttpRequestParser();

    void parse(const char *c_str, size_t len);
};

#endif
