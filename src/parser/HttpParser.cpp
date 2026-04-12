#include "HttpParser.hpp"
#include "Buffer.hpp"
#include "HttpParserBody.hpp"
#include "HttpParserRequestLine.hpp"
#include "HttpParserState.hpp"
#include "HttpRequest.hpp"

#include <cstddef>
#include <cstdio>
#include <cstring>

HttpParser::HttpParser()
    : HttpParserRequestLine(),
      HttpParserState(),
      HttpParserBody()
{
}

void HttpParser::clear()
{

    HttpParserBody::clear();

    HttpParserRequestLine::clear();
    HttpParserState::clear();
}

void HttpParser::parseBuffer(HttpRequest &request)
{
    if (request.complete())
        return;

    Buffer buffer(m_buff);
    while (!buffer.empty())
    {
        switch (m_phase)
        {
            case HttpParserState::P_REQUEST_LINE:
                parseRequestLine(request, buffer);
                break;
            case HttpParserState::P_HEADERS:
                parseHeaders(request, buffer);
                break;
            case HttpParserState::P_BODY:
                parse_body(request, buffer);
                if (m_complete)
                {
                    request.setComplete(true);
                    m_buff.erase(0, buffer.capacity() - buffer.size());
                    return;
                }
                break;
        }
        if (!good())
            return;
    }
    m_buff.clear();
}

void HttpParser::parse(HttpRequest &request, const char *c_str, size_t len)
{
    parseBuffer(request);
    if (request.complete())
    {
        m_buff.append(c_str, len);
        return;
    }
    Buffer buffer(c_str, len);
    while (!buffer.empty())
    {
        switch (m_phase)
        {
            case HttpParserState::P_REQUEST_LINE:
                parseRequestLine(request, buffer);
                break;
            case HttpParserState::P_HEADERS:
                parseHeaders(request, buffer);
                break;
            case HttpParserState::P_BODY:
                parse_body(request, buffer);
                if (m_complete)
                {
                    request.setComplete(true);
                    if (!buffer.empty())
                    {
                        m_buff.append(buffer.current(), buffer.size());
                        buffer.consume(buffer.size());
                    }
                    return;
                }
                break;
        }
    }
    if (!request.good())
        return;
}

HttpParser::~HttpParser()
{
}
