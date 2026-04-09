#include "HttpParserRequestLine.hpp"
#include "State.hpp"

#define CR '\r'
#define LF '\n'

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

HttpParserRequestLine::Action HttpParserRequestLine::req_start(u_char ch)
{
    if (ch == CR || ch == LF)
        return PA_CONTINUE;

    if (!is_valid_method_char(ch))
    {
        setError(error::bad_method);
        return PA_ERROR;
    }

    m_method += ch;
    m_state   = state::s_req_method;
    return PA_CONTINUE;
}

HttpParserRequestLine::Action HttpParserRequestLine::req_method(u_char ch)
{
    if (ch == ' ')
    {
        m_state = state::s_req_spaces_before_uri;
        return PA_CONTINUE;
    }

    if (!is_valid_method_char(ch))
    {
        setError(error::bad_method);
        return PA_ERROR;
    }

    m_method += ch;
    return PA_CONTINUE;
}

HttpParserRequestLine::Action
HttpParserRequestLine::req_spaces_before_uri(u_char ch)
{
    if (ch == '/')
    {
        m_target += ch;
        m_state   = state::s_req_uri_after_slash;
        return PA_CONTINUE;
    }

    char lower = (unsigned char)(ch | 0x20);
    if (lower >= 'a' && lower <= 'z')
    {
        setError(error::unsupported_schema);
        return PA_ERROR;
    }

    if (ch != ' ')
    {
        setError(error::bad_request);
        return PA_ERROR;
    }
    return PA_CONTINUE;
}

HttpParserRequestLine::Action
HttpParserRequestLine::req_uri_after_slash(u_char ch)
{
    if (is_usual(ch))
    {
        m_target += ch;
        m_state   = state::s_req_check_uri;
        return PA_CONTINUE;
    }

    switch (ch)
    {
        case ' ':
            m_state = state::s_req_http_09;
            return PA_CONTINUE;
        case CR:
            m_minor = 9;
            m_state = state::s_req_almost_done;
            return PA_CONTINUE;
        case LF:
            m_minor = 9;
            m_state = state::s_hdr_start;
            return PA_REQUEST_LINE_DONE;
        case '.':
        case '%':
        case '/':
        case '?':
        case '#':
            m_target += ch;
            m_state   = state::s_req_uri;
            return PA_CONTINUE;
        case '+':
            m_target += ch;
            return PA_CONTINUE;
        default:
            if (is_control(ch))
            {
                setError(error::bad_request);
                return PA_ERROR;
            }
            m_target += ch;
            m_state   = state::s_req_check_uri;
            return PA_CONTINUE;
    }
}

HttpParserRequestLine::Action HttpParserRequestLine::req_check_uri(u_char ch)
{
    if (is_usual(ch))
    {
        m_target += ch;
        return PA_CONTINUE;
    }

    switch (ch)
    {
        case '/':
            m_target += ch;
            m_state   = state::s_req_uri_after_slash;
            return PA_CONTINUE;
        case '.':
            m_target += ch;
            return PA_CONTINUE;
        case ' ':
            m_state = state::s_req_http_09;
            return PA_CONTINUE;
        case CR:
            m_minor = 9;
            m_state = state::s_req_almost_done;
            return PA_CONTINUE;
        case LF:
            m_minor = 9;
            m_state = state::s_hdr_start;
            return PA_REQUEST_LINE_DONE;
        case '%':
        case '?':
        case '#':
            m_target += ch;
            m_state   = state::s_req_uri;
            return PA_CONTINUE;
        case '+':
            m_target += ch;
            return PA_CONTINUE;
        default:
            if (is_control(ch))
            {
                setError(error::bad_request);
                return PA_ERROR;
            }
            m_target += ch;
            return PA_CONTINUE;
    }
}

HttpParserRequestLine::Action HttpParserRequestLine::req_uri(u_char ch)
{
    if (is_usual(ch))
    {
        m_target += ch;
        return PA_CONTINUE;
    }

    switch (ch)
    {
        case ' ':
            m_state = state::s_req_http_09;
            return PA_CONTINUE;
        case CR:
            m_minor = 9;
            m_state = state::s_req_almost_done;
            return PA_CONTINUE;
        case LF:
            m_minor = 9;
            m_state = state::s_hdr_start;
            return PA_REQUEST_LINE_DONE;
        case '#':
            m_target += ch;
            return PA_CONTINUE;
        default:
            if (is_control(ch))
            {
                setError(error::bad_request);
                return PA_ERROR;
            }
            m_target += ch;
            return PA_CONTINUE;
    }
}

HttpParserRequestLine::Action HttpParserRequestLine::req_http_09(u_char ch)
{
    switch (ch)
    {
        case ' ':
            return PA_CONTINUE;
        case CR:
            m_minor = 9;
            m_state = state::s_req_almost_done;
            return PA_CONTINUE;
        case LF:
            m_minor = 9;
            m_state = state::s_hdr_start;
            return PA_REQUEST_LINE_DONE;
        case 'H':
            m_state = state::s_req_http_H;
            return PA_CONTINUE;
        default:
            setError(error::bad_request);
            return PA_ERROR;
    }
}

HttpParserRequestLine::Action HttpParserRequestLine::req_http_H(u_char ch)
{
    if (ch != 'T')
    {
        setError(error::bad_request);
        return PA_ERROR;
    }
    m_state = state::s_req_http_HT;
    return PA_CONTINUE;
}

HttpParserRequestLine::Action HttpParserRequestLine::req_http_HT(u_char ch)
{
    if (ch != 'T')
    {
        setError(error::bad_request);
        return PA_ERROR;
    }
    m_state = state::s_req_http_HTT;
    return PA_CONTINUE;
}

HttpParserRequestLine::Action HttpParserRequestLine::req_http_HTT(u_char ch)
{
    if (ch != 'P')
    {
        setError(error::bad_request);
        return PA_ERROR;
    }
    m_state = state::s_req_http_HTTP;
    return PA_CONTINUE;
}

HttpParserRequestLine::Action HttpParserRequestLine::req_http_HTTP(u_char ch)
{
    if (ch != '/')
    {
        setError(error::bad_request);
        return PA_ERROR;
    }
    m_state = state::s_req_first_major_digit;
    return PA_CONTINUE;
}

HttpParserRequestLine::Action
HttpParserRequestLine::req_first_major_digit(u_char ch)
{
    if (ch < '1' || ch > '9')
    {
        setError(error::bad_request);
        return PA_ERROR;
    }

    m_major = ch - '0';

    if (m_major > 1)
    {
        setError(error::bad_version);
        return PA_ERROR;
    }

    m_state = state::s_req_major_digit;
    return PA_CONTINUE;
}

HttpParserRequestLine::Action HttpParserRequestLine::req_major_digit(u_char ch)
{
    if (ch == '.')
    {
        m_state = state::s_req_first_minor_digit;
        return PA_CONTINUE;
    }

    if (ch < '0' || ch > '9')
    {
        setError(error::bad_request);
        return PA_ERROR;
    }

    m_major = m_major * 10 + (ch - '0');

    if (m_major > 1)
    {
        setError(error::bad_version);
        return PA_ERROR;
    }
    return PA_CONTINUE;
}

HttpParserRequestLine::Action
HttpParserRequestLine::req_first_minor_digit(u_char ch)
{
    if (ch < '0' || ch > '9')
    {
        setError(error::bad_version);
        return PA_ERROR;
    }

    m_minor = ch - '0';
    m_state = state::s_req_minor_digit;
    return PA_CONTINUE;
}

HttpParserRequestLine::Action HttpParserRequestLine::req_minor_digit(u_char ch)
{
    if (ch == CR)
    {
        m_state = state::s_req_almost_done;
        return PA_CONTINUE;
    }
    if (ch == LF)
    {
        m_state = state::s_hdr_start;
        return PA_REQUEST_LINE_DONE;
    }
    if (ch == ' ')
    {
        m_state = state::s_req_spaces_after_digit;
        return PA_CONTINUE;
    }

    if (ch < '0' || ch > '9')
    {
        setError(error::bad_request);
        return PA_ERROR;
    }

    if (m_minor > 99)
    {
        setError(error::bad_request);
        return PA_ERROR;
    }

    m_minor = m_minor * 10 + (ch - '0');
    return PA_CONTINUE;
}

HttpParserRequestLine::Action
HttpParserRequestLine::req_spaces_after_digit(u_char ch)
{
    switch (ch)
    {
        case ' ':
            return PA_CONTINUE;
        case CR:
            m_state = state::s_req_almost_done;
            return PA_CONTINUE;
        case LF:
            m_state = state::s_hdr_start;
            return PA_REQUEST_LINE_DONE;
        default:
            setError(error::bad_request);
            return PA_ERROR;
    }
}

HttpParserRequestLine::Action HttpParserRequestLine::req_almost_done(u_char ch)
{
    if (ch != LF)
    {
        setError(error::bad_line_ending);
        return PA_ERROR;
    }
    m_state = state::s_hdr_start;
    return PA_REQUEST_LINE_DONE;
}
