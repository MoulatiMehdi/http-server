#include "HttpParser.hpp"
#include <cstddef>

#define CR    '\r'
#define LF    '\n'

#define ERROR -2

static bool is_usual(u_char ch)
{
    static uint32_t usual[] = {
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */
                    /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0x7fff37d6, /* 0111 1111 1111 1111  0011 0111 1101 0110 */
                    /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
                    /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0x7fffffff, /* 0111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff,
        0xffffffff,
        0xffffffff,
        0xffffffff
    };
    return usual[(unsigned char)ch >> 5] & (1U << (ch & 0x1f));
}

static bool is_valid_method_char(u_char ch)
{
    return (ch >= 'A' && ch <= 'Z') || ch == '_' || ch == '-';
}

static bool is_control(u_char ch)
{
    return ch < 0x20 || ch == 0x7f;
}

void HttpParser::req_start(u_char ch)
{
    if (ch == CR || ch == LF)
        return;

    if (!is_valid_method_char(ch))
    {
        m_error = error::bad_method;
        return;
    }

    m_method += ch;
    m_state   = state::s_req_method;
}

void HttpParser::req_method(u_char ch)
{
    if (ch == ' ')
    {
        m_state = state::s_req_spaces_before_uri;
        return;
    }

    if (!is_valid_method_char(ch))
    {
        m_error = error::bad_method;
        return;
    }

    m_method += ch;
}

void HttpParser::req_spaces_before_uri(u_char ch)
{
    if (ch == '/')
    {
        m_target += ch;
        m_state   = state::s_req_after_slash_in_uri;
        return;
    }

    char lower = (unsigned char)(ch | 0x20);
    if (lower >= 'a' && lower <= 'z')
    {
        m_error = error::unsupported_schema;
        return;
    }

    if (ch != ' ')
    {
        m_error = error::bad_request;
        return;
    }
}

void HttpParser::req_after_slash_in_uri(u_char ch)
{
    if (is_usual(ch))
    {
        m_target += ch;
        m_state   = state::s_req_check_uri;
        return;
    }

    switch (ch)
    {
        case ' ':
            m_state = state::s_req_http_09;
            return;
        case CR:
            m_minor = 9;
            m_state = state::s_req_almost_done;
            return;
        case LF:
            m_minor = 9;
            m_state = state::s_req_done;
        case '.':
        case '%':
        case '/':
        case '?':
        case '#':
            m_target += ch;
            m_state   = state::s_req_uri;
            return;
        case '+':
            m_target += ch;
            return;
        default:
            if (is_control(ch))
            {
                m_error = error::bad_request;
                return;
            }
            m_target += ch;
            m_state   = state::s_req_check_uri;
            return;
    }
}

void HttpParser::req_check_uri(u_char ch)
{
    if (is_usual(ch))
    {
        m_target += ch;
        return;
    }

    switch (ch)
    {
        case '/':
            m_target += ch;
            m_state   = state::s_req_after_slash_in_uri;
            return;
        case '.':
            m_target += ch;
            return;
        case ' ':
            m_state = state::s_req_http_09;
            return;
        case CR:
            m_minor = 9;
            m_state = state::s_req_almost_done;
            return;
        case LF:
            m_minor = 9;
            m_state = state::s_req_done;
        case '%':
        case '?':
        case '#':
            m_target += ch;
            m_state   = state::s_req_uri;
            return;
        case '+':
            m_target += ch;
            return;
        default:
            if (is_control(ch))
            {
                m_error = error::bad_request;
                return;
            }
            m_target += ch;
            return;
    }
}

void HttpParser::req_uri(u_char ch)
{
    if (is_usual(ch))
    {
        m_target += ch;
        return;
    }

    switch (ch)
    {
        case ' ':
            m_state = state::s_req_http_09;
            return;
        case CR:
            m_minor = 9;
            m_state = state::s_req_almost_done;
            return;
        case LF:
            m_minor = 9;
            m_state = state::s_req_done;
        case '#':
            m_target += ch;
            return;
        default:
            if (is_control(ch))
            {
                m_error = error::bad_request;
                return;
            }
            m_target += ch;
            return;
    }
}

void HttpParser::req_http_09(u_char ch)
{
    switch (ch)
    {
        case ' ':
            return;
        case CR:
            m_minor = 9;
            m_state = state::s_req_almost_done;
            return;
        case LF:
            m_minor = 9;
            m_state = state::s_req_done;
        case 'H':
            m_state = state::s_req_http_H;
            return;
        default:
            m_error = error::bad_request;
            return;
    }
}

void HttpParser::req_http_H(u_char ch)
{
    if (ch != 'T')
    {
        m_error = error::bad_request;
        return;
    }
    m_state = state::s_req_http_HT;
}

void HttpParser::req_http_HT(u_char ch)
{
    if (ch != 'T')
    {
        m_error = error::bad_request;
        return;
    }
    m_state = state::s_req_http_HTT;
}

void HttpParser::req_http_HTT(u_char ch)
{
    if (ch != 'P')
    {
        m_error = error::bad_request;
        return;
    }
    m_state = state::s_req_http_HTTP;
}

void HttpParser::req_http_HTTP(u_char ch)
{
    if (ch != '/')
    {
        m_error = error::bad_request;
        return;
    }
    m_state = state::s_req_first_major_digit;
}

void HttpParser::req_first_major_digit(u_char ch)
{
    if (ch < '1' || ch > '9')
    {
        m_error = error::bad_request;
        return;
    }

    m_major = ch - '0';

    if (m_major > 1)
    {
        m_error = error::bad_version;
        return;
    }

    m_state = state::s_req_major_digit;
}

void HttpParser::req_major_digit(u_char ch)
{
    if (ch == '.')
    {
        m_state = state::s_req_first_minor_digit;
        return;
    }

    if (ch < '0' || ch > '9')
    {
        m_error = error::bad_request;
        return;
    }

    m_major = m_major * 10 + (ch - '0');

    if (m_major > 1)
    {
        m_error = error::bad_version;
        return;
    }
}

void HttpParser::req_first_minor_digit(u_char ch)
{
    if (ch < '0' || ch > '9')
    {
        m_error = error::bad_version;
        return;
    }

    m_minor = ch - '0';
    m_state = state::s_req_minor_digit;
}

void HttpParser::req_minor_digit(u_char ch)
{
    if (ch == CR)
    {
        m_state = state::s_req_almost_done;
        return;
    }
    if (ch == LF)
        m_state = state::s_req_done;
    if (ch == ' ')
    {
        m_state = state::s_req_spaces_after_digit;
        return;
    }

    if (ch < '0' || ch > '9')
    {
        m_error = error::bad_request;
        return;
    }

    if (m_minor > 99)
    {
        m_error = error::bad_request;
        return;
    }

    m_minor = m_minor * 10 + (ch - '0');
}

void HttpParser::req_spaces_after_digit(u_char ch)
{
    switch (ch)
    {
        case ' ':
        case CR:
            m_state = state::s_req_almost_done;
        case LF:
            m_state = state::s_req_done;
        default:
            m_error = error::bad_request;
    }
}

void HttpParser::req_almost_done(u_char ch)
{
    if (ch != LF)
    {
        m_error = error::bad_line_ending;
        return;
    }
    m_state = state::s_req_done;
}

// size_t
// HttpParser::request_line(HttpRequest &request, const char *str, size_t len)
// {
//
//     for (size_t i = 0; i < len; i++)
//     {
//         (this->*handlers[m_state])(str[i]);
//
//         if (m_state == state::s_req_done)
//         {
//             m_size += i + 1;
//             request.set_method(string_to_method(m_method));
//             request.set_target(m_target);
//             request.set_version(m_major, m_minor);
//
//             if (request.version() == 9)
//             {
//                 m_error = error::unsupported_version;
//                 return 0;
//             }
//             return i + 1;
//         }
//
//         if (good())
//             return 0;
//     }
//     m_size += len;
//     return len;
// }
