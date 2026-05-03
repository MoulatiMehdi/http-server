#include "HttpResponseParser.hpp"
#include "HttpParserState.hpp"
#include "HttpResponse.hpp"
#include <cstdio>
#include <cstring>

HttpResponseParser::HttpResponseParser(HttpResponse &request)
    : HttpParserState(request),
      response(request),
      m_size(0)

{
    m_phase        = PHASE_HEADERS;
    m_chunked      = false;
    m_discard_body = false;
}

size_t HttpResponseParser::gcount() const
{
    return m_size;
}

void HttpResponseParser::parse(const char *c_str, size_t len)
{
    if (response.complete() || !response.good())
        return;
    Buffer buffer(c_str, len);
    while (!buffer.empty())
    {
        parse_headers(buffer);
        if (!good() || response.complete())
            break;
    }
    m_size = buffer.capacity() - buffer.size();
}

void HttpResponseParser::clear()
{
    m_size = 0;
    HttpParserState::clear();
}

HttpResponseParser::~HttpResponseParser()
{
}
