#include "HttpRequestParser.hpp"
#include "Buffer.hpp"
#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include "ParserError.hpp"
#include "RouteResult.hpp"
#include "Router.hpp"

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <strings.h>

HttpRequestParser::HttpRequestParser(HttpRequest &request)
    : HttpParserState(request),
      m_request(request),
      m_boundary(),
      m_index(0),
      m_response()
{
}

void HttpRequestParser::process_content_type()
{
    m_boundary = request.extract_key("content-type", "boundary");
    if (m_boundary.empty())
        return setError(error::bad_request);
    m_boundary = "--" + m_boundary;
}

void HttpRequestParser::parse(const char *c_str, size_t len)
{
    if (m_request.complete() || !m_request.good())
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
                if (m_request.config.client_max_body_size <
                    m_request.content_length())
                    return setError(error::body_too_large);
                if (route.action == ROUTE_UPLOAD)
                    process_content_type();
                break;
            case HttpParserState::PHASE_BODY:
                m_index = 0;
                parse_body(buffer);
                break;
        }
        if (m_request.complete() || !good())
        {
            m_request.body().close();
            return;
        }
    }
}

HttpRequestParser::~HttpRequestParser()
{
}
