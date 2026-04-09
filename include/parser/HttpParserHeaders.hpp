#ifndef HTTP_PARSER_HEADERS_HPP
#define HTTP_PARSER_HEADERS_HPP
#include "HttpParserState.hpp"
#include "HttpRequest.hpp"

class HttpParserHeaders : virtual public HttpParserState
{
  protected:
    HttpParserHeaders();

    std::string m_header_name;
    std::string m_header_value;

    Action hdr_start(u_char ch);
    Action hdr_name(u_char ch);
    Action hdr_space_before_value(u_char ch);
    Action hdr_value(u_char ch);
    Action hdr_almost_done(u_char ch);
    Action hdr_header_almost_done(u_char ch);

    void processHeaderLine(HttpRequest &request);
    void processHeaders(HttpRequest &request);

  private:
    void handle_content_length(HttpRequest &request);
    void handle_transfer_encoding(HttpRequest &request);
};
#endif
