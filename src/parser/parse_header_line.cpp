
#include "Buffer.hpp"
#include "HttpParserHeaders.hpp"
#include "HttpParserState.hpp"
#include "ParserError.hpp"
#include <string>

#define CR '\r'
#define LF '\n'

static inline u_char to_lower(u_char ch)
{
    static const u_char lowcase[] =
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0-\0\0"
        "0123456789\0\0\0\0\0\0"
        "\0abcdefghijklmnopqrstuvwxyz\0\0\0\0\0"
        "\0abcdefghijklmnopqrstuvwxyz\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
    return lowcase[ch];
}

static inline bool is_valid_name_char(u_char ch)
{
    return to_lower(ch) != 0;
}

HttpParserHeaders::Action HttpParserHeaders::hdr_start(u_char ch)
{
    m_buff.clear();
    m_chunk_size = 0;

    switch (ch)
    {
        case CR:
            m_state = SW_HEADER_ALMOST_DONE;
            return PA_CONTINUE;
        case LF:
            return PA_DONE;
        default:
            m_state  = SW_NAME;
            u_char c = to_lower(ch);
            if (is_valid_name_char(c))
            {
                m_buff += c;
                return PA_CONTINUE;
            }
            setError(error::bad_header_name);
            return PA_ERROR;
    }
    return PA_CONTINUE;
}

HttpParserHeaders::Action HttpParserHeaders::hdr_name(u_char ch)
{
    u_char c = to_lower(ch);

    if (is_valid_name_char(c))
    {
        m_buff += c;
        return PA_CONTINUE;
    }
    if (ch == ':')
    {
        m_chunk_size = m_buff.size();
        m_state      = SW_SPACE_BEFORE_VALUE;
        return PA_CONTINUE;
    }
    if (ch == CR)
    {
        m_state = SW_ALMOST_DONE;
        return PA_CONTINUE;
    }
    if (ch == LF)
    {
        return PA_OK;
    }
    setError(error::bad_header_name);
    return PA_ERROR;
}

HttpParserHeaders::Action HttpParserHeaders::hdr_space_before_value(u_char ch)
{
    switch (ch)
    {
        case ' ':
            return PA_CONTINUE;
        case CR:
            m_state = SW_ALMOST_DONE;
            return PA_CONTINUE;
        case LF:
            return PA_OK;
        case '\0':
            setError(error::bad_header_name);
            return PA_ERROR;
        default:
            m_buff += ch;
            m_state        = SW_VALUE;
            return PA_CONTINUE;
    }
    return PA_CONTINUE;
}

HttpParserHeaders::Action HttpParserHeaders::hdr_value(u_char ch)
{
    switch (ch)
    {
        case CR:
            m_state = SW_ALMOST_DONE;
            return PA_CONTINUE;
        case LF:
            return PA_OK;
        case '\0':
            setError(error::bad_header_value);
            return PA_ERROR;
        default:
            m_buff += ch;
            return PA_CONTINUE;
    }
    return PA_CONTINUE;
}

HttpParserHeaders::Action HttpParserHeaders::hdr_almost_done(u_char ch)
{
    switch (ch)
    {
        case LF:
            return PA_OK;
        case CR:
            return PA_CONTINUE;
        default:
            setError(error::bad_line_ending);
            return PA_ERROR;
    }
    return PA_CONTINUE;
}

HttpParserHeaders::Action HttpParserHeaders::hdr_header_almost_done(u_char ch)
{
    if (ch == LF)
    {
        return PA_DONE;
    }

    setError(error::bad_line_ending);
    return PA_ERROR;
}

void HttpParserHeaders::parseHeaders(HttpRequest &request, Buffer &buff)
{
    while (!buff.empty())
    {
        char   ch     = buff.getc();
        Action action = (this->*handlers[m_state])(ch);

        switch (action)
        {
            case PA_ERROR:
                processError(request);
                return;
            case PA_DONE:
                m_phase = P_BODY;
                m_state = 0;
                processHeaders(request);
                if (!m_discard_body)
                {
                    if (request.body().open_file() < 0)
                        return setError(error::bad_request);
                }
                return;
            case PA_OK:
                m_state = SW_START;
                processHeaderLine(request);
                break;
            case PA_CONTINUE:
                break;
        }
    }
}
