#include "HttpParserState.hpp"
#include "ParserError.hpp"
#include <string>

#define CR '\r'
#define LF '\n'

enum HeaderState
{
    SW_START = 0,
    SW_NAME,
    SW_SPACE_BEFORE_VALUE,
    SW_VALUE,
    SW_ALMOST_DONE,
    SW_HEADER_ALMOST_DONE,
};

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

unsigned int HttpParserState::hdr_start(u_char ch)
{
    m_buff.clear();
    m_chunk_size = 0;

    switch (ch)
    {
        case CR:
            m_state = SW_HEADER_ALMOST_DONE;
            return RES_CONTINUE;
        case LF:
            return RES_HEADER_DONE;
        default:
            m_state  = SW_NAME;
            u_char c = to_lower(ch);
            if (is_valid_name_char(c))
            {
                m_buff += c;
                return RES_CONTINUE;
            }
            setError(error::bad_header_name);
            return RES_ERROR;
    }
    return RES_CONTINUE;
}

unsigned int HttpParserState::hdr_name(u_char ch)
{
    u_char c = to_lower(ch);

    if (is_valid_name_char(c))
    {
        m_buff += c;
        return RES_CONTINUE;
    }
    if (ch == ':')
    {
        m_chunk_size = m_buff.size();
        m_state      = SW_SPACE_BEFORE_VALUE;
        return RES_CONTINUE;
    }
    if (ch == CR)
    {
        m_state = SW_ALMOST_DONE;
        return RES_CONTINUE;
    }
    if (ch == LF)
    {
        return RES_HEADER_LINE_DONE;
    }
    setError(error::bad_header_name);
    return RES_ERROR;
}

unsigned int HttpParserState::hdr_space_before_value(u_char ch)
{
    switch (ch)
    {
        case ' ':
            return RES_CONTINUE;
        case CR:
            m_state = SW_ALMOST_DONE;
            return RES_CONTINUE;
        case LF:
            return RES_HEADER_LINE_DONE;
        case '\0':
            setError(error::bad_header_name);
            return RES_ERROR;
        default:
            m_buff  += ch;
            m_state  = SW_VALUE;
            return RES_CONTINUE;
    }
    return RES_CONTINUE;
}

unsigned int HttpParserState::hdr_value(u_char ch)
{
    switch (ch)
    {
        case CR:
            m_state = SW_ALMOST_DONE;
            return RES_CONTINUE;
        case LF:
            return RES_HEADER_LINE_DONE;
        case '\0':
            setError(error::bad_header_value);
            return RES_ERROR;
        default:
            m_buff += ch;
            return RES_CONTINUE;
    }
    return RES_CONTINUE;
}

unsigned int HttpParserState::hdr_almost_done(u_char ch)
{
    switch (ch)
    {
        case LF:
            return RES_HEADER_LINE_DONE;
        case CR:
            return RES_CONTINUE;
        default:
            setError(error::bad_line_ending);
            return RES_ERROR;
    }
    return RES_CONTINUE;
}

unsigned int HttpParserState::hdr_header_almost_done(u_char ch)
{
    if (ch == LF)
    {
        return RES_HEADER_DONE;
    }

    setError(error::bad_line_ending);
    return RES_ERROR;
}
