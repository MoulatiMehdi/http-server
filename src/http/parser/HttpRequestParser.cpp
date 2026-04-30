#include "HttpRequestParser.hpp"
#include "Buffer.hpp"
#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include "ParserError.hpp"

#include <cstddef>
#include <cstdio>
#include <cstring>

HttpRequestParser::HttpRequestParser(HttpRequest &request)
    : HttpParserState(request),
      request(request)
{
}

void HttpRequestParser::parse(const char *c_str, size_t len)
{
    if (request.complete() || !request.good())
        return;
    Buffer buffer(c_str, len);
    while (!buffer.empty())
    {
        switch (m_phase)
        {
            case HttpParserState::PHASE_REQUEST_LINE:
                parse_request_line(buffer);
                break;
            case HttpParserState::PHASE_HEADERS:
                parse_headers(buffer);
                if (request.config.client_max_body_size <
                    request.content_length())
                    setError(error::body_too_large);
                break;
            case HttpParserState::PHASE_BODY:
                parse_body(buffer);
                break;
        }
        if (request.complete() || !good())
        {
            request.body().close();
            return;
        }
    }
}

HttpRequestParser::~HttpRequestParser()
{
}
