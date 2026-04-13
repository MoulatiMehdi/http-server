#ifndef HTTP_PARSER_HEADERS_HPP
#define HTTP_PARSER_HEADERS_HPP
#include "Buffer.hpp"
#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include <sys/types.h>

class HttpParserHeaders : virtual public HttpParserState
{
  private:
    enum HeaderState
    {
        SW_START = 0,
        SW_NAME,
        SW_SPACE_BEFORE_VALUE,
        SW_VALUE,
        SW_ALMOST_DONE,
        SW_HEADER_ALMOST_DONE,
    };

    typedef Action (HttpParserHeaders::*Handler)(u_char);

    const static Handler handlers[6];

  protected:
    HttpParserHeaders();

    Action hdr_start(u_char ch);
    Action hdr_name(u_char ch);
    Action hdr_space_before_value(u_char ch);
    Action hdr_value(u_char ch);
    Action hdr_almost_done(u_char ch);
    Action hdr_header_almost_done(u_char ch);

    void parseHeaders(HttpRequest &request, Buffer &buff);
    void processHeaderLine(HttpRequest &request);
    void processHeaders(HttpRequest &request);

  private:
    void handle_content_length(HttpRequest &request);
    void handle_transfer_encoding(HttpRequest &request);
};
#endif
