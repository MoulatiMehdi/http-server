#ifndef HTTP_PARSER_REQUEST_LINE_HPP
#define HTTP_PARSER_REQUEST_LINE_HPP

#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include <string>

class HttpParserRequestLine : virtual public HttpParserState
{
  protected:
    HttpParserRequestLine();
    ~HttpParserRequestLine();

    std::string m_target;
    std::string m_method;

    unsigned short m_major;
    unsigned short m_minor;

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

    void processRequestLine(HttpRequest &request);
};
#endif
