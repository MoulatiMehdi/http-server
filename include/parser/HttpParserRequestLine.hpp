#ifndef HTTP_PARSER_REQUEST_LINE_HPP
#define HTTP_PARSER_REQUEST_LINE_HPP

#include "Buffer.hpp"
#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include <string>
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

    typedef Action (HttpParserRequestLine::*Handler)(u_char);
    static Handler handlers[17];

  protected:
    HttpParserRequestLine();
    ~HttpParserRequestLine();

    std::string m_target;
    std::string m_method;

    unsigned short m_major;
    unsigned short m_minor;

  private:
    Action req_start(u_char ch);
    Action req_method(u_char ch);
    Action req_spaces_before_uri(u_char ch);
    Action req_uri_after_slash(u_char ch);
    Action req_check_uri(u_char ch);
    Action req_uri(u_char ch);
    Action req_http_09(u_char ch);
    Action req_http_H(u_char ch);
    Action req_http_HT(u_char ch);
    Action req_http_HTT(u_char ch);
    Action req_http_HTTP(u_char ch);
    Action req_first_major_digit(u_char ch);
    Action req_major_digit(u_char ch);
    Action req_first_minor_digit(u_char ch);
    Action req_minor_digit(u_char ch);
    Action req_spaces_after_digit(u_char ch);
    Action req_almost_done(u_char ch);

  protected:
    void clear();
    void processRequestLine(HttpRequest &request);
    void parseRequestLine(HttpRequest &request, Buffer &buff);
};
#endif
