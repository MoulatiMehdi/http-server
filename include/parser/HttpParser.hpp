#ifndef HTTP_PARSER_HPP
#define HTTP_PARSER_HPP

#include "HttpParserBody.hpp"
#include "HttpParserHeaders.hpp"
#include "HttpParserRequestLine.hpp"
#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include <cstddef>

class HttpParser : HttpParserRequestLine, HttpParserHeaders, HttpParserBody
{
  public:
    HttpParser();
    HttpParser(const HttpParser &other);
    HttpParser operator=(const HttpParser &other);
    ~HttpParser();

    void clear();
    using HttpParserState::error;
    using HttpParserState::good;
    using HttpParserState::state;

    void parse(HttpRequest &request, const char *c_str, size_t len);
    void processRequest(HttpRequest &request, Action &action);

  private:
    using Handler = Action (HttpParser::*)(u_char);
    static Handler handlers[30];
};

#endif
