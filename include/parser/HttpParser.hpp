#ifndef HTTP_PARSER_HPP
#define HTTP_PARSER_HPP

#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include <cstddef>

class HttpParser : public HttpParserState
{
  public:
    HttpParser(HttpRequest &request);
    ~HttpParser();

    using HttpParserState::good;
    void parse(const char *c_str, size_t len);
};

#endif
