#ifndef HTTP_PARSER_REQUEST_LINE_HPP
#define HTTP_PARSER_REQUEST_LINE_HPP

#include "Buffer.hpp"
#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include <sys/types.h>

class HttpParserRequestLine : virtual public HttpParserState
{
  private:
    enum RequestLineState
    {
        SW_START = 0,
        SW_METHOD,
        SW_SPACES_BEFORE_URI,
        SW_URI_AFTER_SLASH,
        SW_CHECK_URI,
        SW_URI,
        SW_HTTP_09,
        SW_HTTP_H,
        SW_HTTP_HT,
        SW_HTTP_HTT,
        SW_HTTP_HTTP,
        SW_FIRST_MAJOR_DIGIT,
        SW_MAJOR_DIGIT,
        SW_FIRST_MINOR_DIGIT,
        SW_MINOR_DIGIT,
        SW_SPACES_AFTER_DIGIT,
        SW_ALMOST_DONE,
    };

    typedef Action (HttpParserRequestLine::*Handler)(HttpRequest &, u_char);
    static Handler handlers[17];

  protected:
    HttpParserRequestLine();
    ~HttpParserRequestLine();


  private:
    Action req_start(HttpRequest &request, u_char ch);
    Action req_method(HttpRequest &request, u_char ch);
    Action req_spaces_before_uri(HttpRequest &request, u_char ch);
    Action req_uri_after_slash(HttpRequest &request, u_char ch);
    Action req_check_uri(HttpRequest &request, u_char ch);
    Action req_uri(HttpRequest &request, u_char ch);
    Action req_http_09(HttpRequest &request, u_char ch);
    Action req_http_H(HttpRequest &request, u_char ch);
    Action req_http_HT(HttpRequest &request, u_char ch);
    Action req_http_HTT(HttpRequest &request, u_char ch);
    Action req_http_HTTP(HttpRequest &request, u_char ch);
    Action req_first_major_digit(HttpRequest &request, u_char ch);
    Action req_major_digit(HttpRequest &request, u_char ch);
    Action req_first_minor_digit(HttpRequest &request, u_char ch);
    Action req_minor_digit(HttpRequest &request, u_char ch);
    Action req_spaces_after_digit(HttpRequest &request, u_char ch);
    Action req_almost_done(HttpRequest &request, u_char ch);

  protected:
    void processRequestLine(HttpRequest &request);
    void parseRequestLine(HttpRequest &request, Buffer &buff);
};
#endif
