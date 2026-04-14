#include "HttpParser.hpp"
#include "Buffer.hpp"
#include "HttpParserState.hpp"
#include "HttpRequest.hpp"

#include <cstddef>
#include <cstdio>
#include <cstring>

HttpParser::HttpParser(HttpRequest &request) : HttpParserState(request)
{
}

void HttpParser::parse(const char *c_str, size_t len)
{
    if (request.complete())
        return;
    Buffer buffer(c_str, len);
    while (!buffer.empty())
    {
        switch (m_phase)
        {
            case HttpParserState::P_REQUEST_LINE:
                parse_request_line(buffer);
                break;
            case HttpParserState::P_HEADERS:
                parse_headers(buffer);
                break;
            case HttpParserState::P_BODY:
                parse_body(buffer);
                if (request.complete())
                {
                    if (!buffer.empty())
                        buffer.consume(buffer.size());
                    return;
                }
                break;
        }
        if (!good())
            return;
    }
}

HttpParser::~HttpParser()
{
}
