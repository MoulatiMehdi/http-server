
#include "HttpParserBody.hpp"
#include "Buffer.hpp"
#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include "ParserError.hpp"
#include <climits>
#include <cstddef>

#define CR '\r'
#define LF '\n'

HttpParserBody::HttpParserBody()
    : HttpParserState(),
      m_chunk_size(0),
      m_chunk_max_size(0)
{
}

void HttpParserBody::parse_body(HttpRequest &request, Buffer &buffer)
{
    size_t size = 0;

    if (m_chunked)
        parse_body_chunk(request, buffer);
    else if (request.content_length() > 0)
        parse_body_length(request, buffer);
    else
        m_complete = true;
}

void HttpParserBody::parse_body_chunk(HttpRequest &request, Buffer &buffer)
{

    while (!buffer.empty())
    {
        const char ch = buffer.getc();
        const char c  = ch | 0x20;

        switch (m_state)
        {
            case SW_CHUNK_START:
                if (ch >= '0' && ch <= '9')
                {
                    m_state          = SW_CHUNK_SIZE;
                    m_chunk_max_size = ch - '0';
                    break;
                }
                if (c >= 'a' && c <= 'f')
                {
                    m_state          = SW_CHUNK_SIZE;
                    m_chunk_max_size = c - 'a' + 10;
                    break;
                }

                return setError(ParserError::bad_request);
            case SW_CHUNK_SIZE:
                if (m_chunk_max_size > LONG_MAX / 16)
                {
                    return setError(ParserError::bad_request);
                }
                if (ch >= '0' && ch <= '9')
                {
                    m_chunk_max_size = m_chunk_max_size * 16 + (ch - '0');
                    break;
                }
                if (c >= 'a' && c <= 'f')
                {
                    m_chunk_max_size = m_chunk_max_size * 16 + (c - 'a' + 10);
                    break;
                }
                if (m_chunk_max_size == 0)
                {
                    switch (ch)
                    {
                        case CR:
                            m_state = SW_LAST_CHUNK_SIZE_ALMOST_DONE;
                            break;
                        case LF:
                            m_state = SW_LAST_CHUNK_DATA_ALMOST_DONE;
                            break;
                        default:
                            return setError(ParserError::bad_request);
                    }
                    break;
                }

                switch (ch)
                {
                    case CR:
                        m_state = SW_CHUNK_SIZE_ALMOST_DONE;
                        break;
                    case LF:
                        m_state = SW_CHUNK_DATA;
                        break;
                    default:
                        return setError(ParserError::bad_request);
                }

                break;
            case SW_CHUNK_SIZE_ALMOST_DONE:
                if (ch == LF)
                {
                    m_state = SW_CHUNK_DATA;
                    break;
                }
                return setError(ParserError::bad_request);
            case SW_CHUNK_DATA:
                if (request.body().append(ch) < 0)
                    return setError(ParserError::bad_request);

                m_chunk_size++;
                if (m_chunk_size == m_chunk_max_size)
                {
                    m_state          = SW_AFTER_DATA;
                    m_chunk_max_size = 0;
                    m_chunk_size     = 0;
                }
                break;

            case SW_AFTER_DATA:
                switch (ch)
                {
                    case CR:
                        m_state = SW_AFTER_DATA_ALMOST_DONE;
                        break;
                    case LF:
                        m_state = SW_CHUNK_START;
                        break;
                    default:
                        return setError(ParserError::bad_request);
                }
                break;

            case SW_AFTER_DATA_ALMOST_DONE:
                if (ch == LF)
                {
                    m_state = SW_CHUNK_START;
                    break;
                }
                return setError(ParserError::bad_request);

            case SW_LAST_CHUNK_SIZE_ALMOST_DONE:
                if (ch == LF)
                {
                    m_state = SW_LAST_CHUNK_DATA_ALMOST_DONE;
                    break;
                }
                return setError(ParserError::bad_request);

            case SW_LAST_CHUNK_DATA_ALMOST_DONE:
                switch (ch)
                {
                    case CR:
                        m_state = SW_BODY_ALMOST_DONE;
                        break;
                    case LF:
                        return;
                    default:
                        return setError(ParserError::bad_request);
                }
                break;

            case SW_BODY_ALMOST_DONE:
                if (ch == LF)
                {
                    m_complete = true;
                    return;
                }
                return setError(ParserError::bad_request);
        }
    }
}

void HttpParserBody::parse_body_length(HttpRequest &request, Buffer &buffer)
{
    const size_t content_length = request.content_length();

    if (content_length > m_buff.size() && !buffer.empty())
    {
        size_t size = std::min(content_length - m_buff.size(), buffer.size());
        if (request.body().append(buffer.current(), size) < 0)
            return setError(ParserError::bad_request);
        buffer.consume(size);
    }
    m_complete = request.content_length() == request.body().size();
}

void HttpParserBody::clear()
{
    m_chunk_max_size = 0;
    m_chunk_size     = 0;
}

HttpParserBody::~HttpParserBody()
{
}
