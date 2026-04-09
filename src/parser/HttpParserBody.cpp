
#include "HttpParserBody.hpp"
#include "Error.hpp"
#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include <climits>
#include <cstddef>

#define CR '\r'
#define LF '\n'

HttpParserBody::HttpParserBody()
    : HttpParserState(),
      m_body_size(0),
      m_buff(),
      m_chunk_max_size(0),
      m_chunk_value(),
      m_chunk_state(ChunkState::sw_chunk_start)
{
}

void HttpParserBody::parse_body(
    HttpRequest &request, const char *str, size_t len
)
{
    size_t size = 0;

    if (m_chunked)
        size = parse_body_chunk(request, str, len);
    else if (request.content_length() > 0)
        size = parse_body_length(request, str, len);
    else
        m_complete = true;
    if (m_complete)
    {
        if (size < len)
            m_buff.append(&str[size], len - size);
    }
}

size_t HttpParserBody::parse_body_chunk(
    HttpRequest &request, const char *str, size_t len
)
{

    for (int i = 0; i < len; i++)
    {
        const char ch = str[i];
        const char c  = ch | 0x20;

        switch (m_chunk_state)
        {
            case sw_chunk_start:
                if (ch >= '0' && ch <= '9')
                {
                    m_chunk_state    = sw_chunk_size;
                    m_chunk_max_size = ch - '0';
                    break;
                }
                if (c >= 'a' && c <= 'f')
                {
                    m_chunk_state    = sw_chunk_size;
                    m_chunk_max_size = c - 'a' + 10;
                    break;
                }

                setError(error::bad_request);
                return 0;
            case sw_chunk_size:
                if (m_chunk_max_size > LONG_MAX / 16)
                {
                    setError(error::bad_request);
                    return 0;
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
                            m_chunk_state = sw_last_chunk_size_almost_done;
                            break;
                        case LF:
                            m_chunk_state = sw_last_chunk_data_almost_done;
                            break;
                        default:
                            setError(error::bad_request);
                            return 0;
                    }
                    break;
                }

                switch (ch)
                {
                    case CR:
                        m_chunk_state = sw_chunk_size_almost_done;
                        break;
                    case LF:
                        m_chunk_state = sw_chunk_data;
                        break;
                    default:
                        setError(error::bad_request);
                        return 0;
                }

                break;
            case sw_chunk_size_almost_done:
                if (ch == LF)
                {
                    m_chunk_state = sw_chunk_data;
                    break;
                }
                setError(error::bad_request);
                return 0;
            case sw_chunk_data:
                if (!m_discard_body)
                {
                    request.body_file_ostream() << ch;
                }
                m_chunk_size++;
                if (m_chunk_size == m_chunk_max_size)
                {
                    m_body_size += m_chunk_size;
                    if (!m_discard_body)
                        request.body_file_ostream().flush();
                    m_chunk_state    = sw_after_data;
                    m_chunk_max_size = 0;
                    m_chunk_size     = 0;
                }
                break;

            case sw_after_data:
                switch (ch)
                {
                    case CR:
                        m_chunk_state = sw_after_data_almost_done;
                        break;
                    case LF:
                        m_chunk_state = sw_chunk_start;
                        break;
                    default:
                        setError(error::bad_request);
                        return 0;
                }
                break;

            case sw_after_data_almost_done:
                if (ch == LF)
                {
                    m_chunk_state = sw_chunk_start;
                    break;
                }
                setError(error::bad_request);
                return 0;

            case sw_last_chunk_size_almost_done:
                if (ch == LF)
                {
                    m_chunk_state = sw_last_chunk_data_almost_done;
                    break;
                }
                setError(error::bad_request);
                return 0;

            case sw_last_chunk_data_almost_done:
                switch (ch)
                {
                    case CR:
                        m_chunk_state = sw_body_almost_done;
                        break;
                    case LF:
                        return i + 1;
                    default:
                        setError(error::bad_request);
                        return 0;
                }
                break;

            case sw_body_almost_done:
                if (ch == LF)
                {
                    m_complete = true;
                    return i + 1;
                }
                setError(error::bad_request);
                return 0;
        }
    }
    return len;
}

size_t HttpParserBody::parse_body_length(
    HttpRequest &request, const char *str, size_t len
)
{
    size_t         size           = 0;
    std::ofstream &ofs            = request.body_file_ostream();
    const size_t   content_length = request.content_length();

    if (content_length > m_body_size && len > 0)
    {
        size = std::min(content_length - m_body_size, len);
        if (!m_discard_body)
        {
            ofs.write(str, size);
            ofs.flush();
        }
        m_body_size += size;
    }
    m_complete = request.content_length() == m_body_size;

    return size;
}

HttpParserBody::~HttpParserBody()
{
}
