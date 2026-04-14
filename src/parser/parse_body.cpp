#include "Buffer.hpp"
#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include "ParserError.hpp"
#include <algorithm>
#include <climits>
#include <cstddef>
#include <sys/types.h>

#define CR '\r'
#define LF '\n'

enum ChunkState
{
    SW_CHUNK_START = 0,
    SW_CHUNK_SIZE,
    SW_CHUNK_SIZE_ALMOST_DONE,
    SW_CHUNK_DATA,
    SW_AFTER_DATA,
    SW_AFTER_DATA_ALMOST_DONE,
    SW_LAST_CHUNK_SIZE_ALMOST_DONE,
    SW_LAST_CHUNK_DATA_ALMOST_DONE,
    SW_BODY_ALMOST_DONE,
};

void HttpParserState::parse_body(Buffer &buffer)
{
    if (m_chunked)
        parse_body_by_chunk(buffer);
    else if (request.content_length() > 0)
        parse_body_by_length(buffer);
    else
        request.setComplete(true);
}

void HttpParserState::parse_body_by_chunk(Buffer &buffer)
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

                return setError(error::bad_request);
            case SW_CHUNK_SIZE:
                if (m_chunk_max_size > LONG_MAX / 16)
                {
                    return setError(error::bad_request);
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
                            return setError(error::bad_request);
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
                        return setError(error::bad_request);
                }

                break;
            case SW_CHUNK_SIZE_ALMOST_DONE:
                if (ch == LF)
                {
                    m_state = SW_CHUNK_DATA;
                    break;
                }
                return setError(error::bad_request);
            case SW_CHUNK_DATA:
                if (request.body().append(ch) < 0)
                    return setError(error::bad_request);

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
                        return setError(error::bad_request);
                }
                break;

            case SW_AFTER_DATA_ALMOST_DONE:
                if (ch == LF)
                {
                    m_state = SW_CHUNK_START;
                    break;
                }
                return setError(error::bad_request);

            case SW_LAST_CHUNK_SIZE_ALMOST_DONE:
                if (ch == LF)
                {
                    m_state = SW_LAST_CHUNK_DATA_ALMOST_DONE;
                    break;
                }
                return setError(error::bad_request);

            case SW_LAST_CHUNK_DATA_ALMOST_DONE:
                switch (ch)
                {
                    case CR:
                        m_state = SW_BODY_ALMOST_DONE;
                        break;
                    case LF:
                        return;
                    default:
                        return setError(error::bad_request);
                }
                break;

            case SW_BODY_ALMOST_DONE:
                if (ch == LF)
                {
                    request.setComplete(true);
                    return;
                }
                return setError(error::bad_request);
        }
    }
}

void HttpParserState::parse_body_by_length(Buffer &buffer)
{
    const size_t content_length = request.content_length();
    const size_t body_size      = request.body().size();

    if (content_length > body_size && !buffer.empty())
    {
        size_t size = std::min(content_length - body_size, buffer.size());
        if (request.body().append(buffer.current(), size) < 0)
            return setError(error::bad_request);
        buffer.consume(size);
    }
    if (request.content_length() < request.body().size())
        setError(error::stale_parser);
    else if (request.content_length() == request.body().size())
        request.setComplete(true);
}
