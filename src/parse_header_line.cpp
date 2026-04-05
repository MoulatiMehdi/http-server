
#include "Error.hpp"
#include "HttpParser.hpp"
#include "HttpRequest.hpp"
#include <cstddef>
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

void HttpParser::hdr_start(u_char ch)
{
    m_header_name.clear();
    m_header_value.clear();
    m_invalid_header = 0;

    switch (ch)
    {
        case CR:
            m_state = state::s_hdr_header_almost_done;
            break;
        case LF:
            m_state = state::s_hdr_done;
            break;
        default:
            m_state  = state::s_hdr_name;
            u_char c = to_lower(ch);
            if (is_valid_name_char(c))
            {
                m_header_name += c;
                break;
            }
            if (ch <= 0x20 || ch == 0x7f || ch == ':')
            {
                m_error = error::bad_header_name;
                break;
            }
            m_invalid_header = true;
            break;
    }
}

void HttpParser::hdr_name(u_char ch)
{
    u_char c = to_lower(ch);

    if (is_valid_name_char(c))
    {
        m_header_name += c;
        return;
    }
    if (ch == ':')
    {
        m_state = state::s_hdr_space_before_value;
        return;
    }
    if (ch == CR)
    {
        m_state = state::s_hdr_almost_done;
        return;
    }
    if (ch == LF)
    {
        m_state = state::s_hdr_line_done;
        return;
    }
    if (ch <= 0x20 || ch == 0x7f)
    {
        m_error = error::bad_header_name;
        return;
    }
    m_invalid_header = true;
}

void HttpParser::hdr_space_before_value(u_char ch)
{
    switch (ch)
    {
        case ' ':
            break;
        case CR:
            m_state = state::s_hdr_almost_done;
            break;
        case LF:
            m_state = state::s_hdr_line_done;
            break;
        case '\0':
            m_error = error::bad_header_name;
            break;
        default:
            m_header_value += ch;
            m_state         = state::s_hdr_value;
            break;
    }
}

void HttpParser::hdr_value(u_char ch)
{
    switch (ch)
    {
        case CR:
            m_state = state::s_hdr_almost_done;
            break;
        case LF:
            m_state = state::s_hdr_line_done;
            break;
        case '\0':
            m_error = error::bad_header_value;
            break;
        default:
            m_header_value += ch;
            break;
    }
}

void HttpParser::hdr_almost_done(u_char ch)
{
    switch (ch)
    {
        case LF:
            m_state = state::s_hdr_line_done;
            break;
        case CR:
            break;
        default:
            m_error = error::bad_line_ending;
            break;
    }
}

void HttpParser::hdr_header_almost_done(u_char ch)
{
    if (ch == LF)
        m_state = state::s_hdr_done;
    else
        m_error = error::bad_line_ending;
}

// size_t
// HttpParser::header_line(HttpRequest &request, const char *str, size_t len)
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
//         if (m_state == state::s_hdr_line_done)
//         {
//             request.setHeader(m_header_name, m_header_value);
//             m_state = state::s_hdr_start;
//             return i + 1;
//         }
//
//         if (m_state == state::s_hdr_done)
//             return i + 1;
//     }
//
//     return len;
// }
