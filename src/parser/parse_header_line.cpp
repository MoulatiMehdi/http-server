
#include "Error.hpp"
#include "HttpParserHeaders.hpp"
#include <string>

#define CR '\r'
#define LF '\n'

static inline u_char to_lower(u_char ch)
{
    static u_char lowcase[] =
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0-\0\0"
        "0123456789\0\0\0\0\0\0"
        "\0abcdefghijklmnopqrstuvwxyz\0\0\0\0_"
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
    m_header_name.clear();
    m_header_value.clear();
    m_invalid_header = 0;

    switch (ch)
    {
        case CR:
            m_state = state::s_hdr_header_almost_done;
            return PA_CONTINUE;
        case LF:
            m_state = state::s_body_start;
            return PA_HEADER_DONE;
        default:
            m_state  = state::s_hdr_name;
            u_char c = to_lower(ch);
            if (is_valid_name_char(c))
            {
                m_header_name += c;
                return PA_CONTINUE;
            }
            if (ch <= 0x20 || ch == 0x7f || ch == ':')
            {
                setError(error::bad_header_name);
                return PA_ERROR;
            }
            m_invalid_header = true;
            return PA_CONTINUE;
    }
    return PA_CONTINUE;
}

HttpParserHeaders::Action HttpParserHeaders::hdr_name(u_char ch)
{
    u_char c = to_lower(ch);

    if (is_valid_name_char(c))
    {
        m_header_name += c;
        return PA_CONTINUE;
    }
    if (ch == ':')
    {
        m_state = state::s_hdr_space_before_value;
        return PA_CONTINUE;
    }
    if (ch == CR)
    {
        m_state = state::s_hdr_almost_done;
        return PA_CONTINUE;
    }
    if (ch == LF)
    {
        m_state = state::s_hdr_start;
        return PA_HEADER_LINE_DONE;
    }
    if (ch <= 0x20 || ch == 0x7f)
    {
        setError(error::bad_header_name);
        return PA_ERROR;
    }
    m_invalid_header = true;

    return PA_CONTINUE;
}

HttpParserHeaders::Action
HttpParserHeaders::hdr_space_before_value(u_char ch)
{
    switch (ch)
    {
        case ' ':
            return PA_CONTINUE;
        case CR:
            m_state = state::s_hdr_almost_done;
            return PA_CONTINUE;
        case LF:
            m_state = state::s_hdr_start;
            return PA_HEADER_LINE_DONE;
        case '\0':
            setError(error::bad_header_name);
            return PA_ERROR;
        default:
            m_header_value += ch;
            m_state         = state::s_hdr_value;
            return PA_CONTINUE;
    }
    return PA_CONTINUE;
}

HttpParserHeaders::Action HttpParserHeaders::hdr_value(u_char ch)
{
    switch (ch)
    {
        case CR:
            m_state = state::s_hdr_almost_done;
            return PA_CONTINUE;
        case LF:
            m_state = state::s_hdr_start;
            return PA_HEADER_LINE_DONE;
        case '\0':
            setError(error::bad_header_value);
            return PA_ERROR;
        default:
            m_header_value += ch;
            return PA_CONTINUE;
    }
    return PA_CONTINUE;
}

HttpParserHeaders::Action HttpParserHeaders::hdr_almost_done(u_char ch)
{
    switch (ch)
    {
        case LF:
            m_state = state::s_hdr_start;
            return PA_HEADER_LINE_DONE;
        case CR:
            return PA_CONTINUE;
        default:
            setError(error::bad_line_ending);
            return PA_ERROR;
    }
    return PA_CONTINUE;
}

HttpParserHeaders::Action
HttpParserHeaders::hdr_header_almost_done(u_char ch)
{
    if (ch == LF)
    {
        m_state = state::s_body_start;
        return PA_HEADER_DONE;
    }

    setError(error::bad_line_ending);
    return PA_ERROR;
}

// size_t
// HttpParserHeaderLine::header_line(HttpRequest &request, const char *str,
// size_t len)
// {
//
//     if (good())
//         return 0;
//
//     for (size_t i = 0; i < len; i++)
//     {
//         const u_char ch = static_cast<u_char>(str[i]);
//
//         (this->*handlers[m_state])(ch);
//
//         if (good())
//             return 0;
//
//         if (m_state == state::s_hdr_start)
//         {
//             request.setHeader(m_header_name, m_header_value);
//             m_state = state::s_hdr_start;
//             return i + 1;
//         }
//
//         if (m_state == state::s_body_start)
//             return i + 1;
//     }
//
//     return len;
// }
