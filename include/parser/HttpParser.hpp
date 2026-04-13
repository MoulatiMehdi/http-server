#ifndef HTTP_PARSER_HPP
#define HTTP_PARSER_HPP

#include "HttpParserBody.hpp"
#include "HttpParserHeaders.hpp"
#include "HttpParserRequestLine.hpp"
#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include <cstddef>

class HttpParser :
    public HttpParserRequestLine,
    public HttpParserHeaders,
    public HttpParserBody
{
  private:
    void parseBuffer(HttpRequest &request);

  public:
    HttpParser();
    HttpParser(const HttpParser &other);
    HttpParser operator=(const HttpParser &other);
    ~HttpParser();

    using HttpParserState::clear;
    using HttpParserState::good;
    void parse(HttpRequest &request, const char *c_str, size_t len);
};

#endif
