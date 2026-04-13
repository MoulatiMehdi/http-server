#include "HttpParserRequestLine.hpp"
#include "HttpParserState.hpp"

#define CR '\r'
#define LF '\n'

static bool is_usual(u_char ch)
{
    static unsigned int usual[] = {
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

HttpParserRequestLine::Action
HttpParserRequestLine::req_start(HttpRequest &, u_char ch)
{
    if (ch == CR || ch == LF)
        return PA_CONTINUE;

    if (!is_valid_method_char(ch))
    {
        setError(error::bad_method);
        return PA_ERROR;
    }

    m_buff  += ch;
    m_state  = SW_METHOD;
    return PA_CONTINUE;
}

HttpParserRequestLine::Action
HttpParserRequestLine::req_method(HttpRequest &request, u_char ch)
{
    if (ch == ' ')
    {
        request.setMethod(m_buff);
        if (request.method() == method::UNKNOWN)
        {
            setError(error::unsupported_method);
            return PA_ERROR;
        }
        m_buff.clear();
        m_state = SW_SPACES_BEFORE_URI;
        return PA_CONTINUE;
    }

    if (!is_valid_method_char(ch))
    {
        setError(error::bad_method);
        return PA_ERROR;
    }
    m_buff += ch;
    return PA_CONTINUE;
}

HttpParserRequestLine::Action
HttpParserRequestLine::req_spaces_before_uri(HttpRequest &, u_char ch)
{
    if (ch == '/')
    {
        m_buff  += ch;
        m_state  = SW_URI_AFTER_SLASH;
        return PA_CONTINUE;
    }

    if (ch != ' ')
    {
        setError(error::bad_request);
        return PA_ERROR;
    }
    return PA_CONTINUE;
}

HttpParserRequestLine::Action
HttpParserRequestLine::req_uri_after_slash(HttpRequest &, u_char ch)
{
    if (is_usual(ch))
    {
        m_buff  += ch;
        m_state  = SW_CHECK_URI;
        return PA_CONTINUE;
    }

    switch (ch)
    {
        case ' ':
            m_state = SW_HTTP_09;
            return PA_CONTINUE;
        case CR:
            m_minor = 9;
            m_state = SW_ALMOST_DONE;
            return PA_CONTINUE;
        case LF:
            m_minor = 9;
            return PA_DONE;
        case '.':
        case '%':
        case '/':
        case '?':
        case '#':
            m_buff  += ch;
            m_state  = SW_URI;
            return PA_CONTINUE;
        case '+':
            m_buff += ch;
            return PA_CONTINUE;
        default:
            if (is_control(ch))
            {
                setError(error::bad_request);
                return PA_ERROR;
            }
            m_buff  += ch;
            m_state  = SW_CHECK_URI;
            return PA_CONTINUE;
    }
}

HttpParserRequestLine::Action
HttpParserRequestLine::req_check_uri(HttpRequest &, u_char ch)
{
    if (is_usual(ch))
    {
        m_buff += ch;
        return PA_CONTINUE;
    }

    switch (ch)
    {
        case '/':
            m_buff  += ch;
            m_state  = SW_URI_AFTER_SLASH;
            return PA_CONTINUE;
        case '.':
            m_buff += ch;
            return PA_CONTINUE;
        case ' ':
            m_state = SW_HTTP_09;
            return PA_CONTINUE;
        case CR:
            m_minor = 9;
            m_state = SW_ALMOST_DONE;
            return PA_CONTINUE;
        case LF:
            m_minor = 9;
            return PA_DONE;
        case '%':
        case '?':
        case '#':
            m_buff  += ch;
            m_state  = SW_URI;
            return PA_CONTINUE;
        case '+':
            m_buff += ch;
            return PA_CONTINUE;
        default:
            if (is_control(ch))
            {
                setError(error::bad_request);
                return PA_ERROR;
            }
            m_buff += ch;
            return PA_CONTINUE;
    }
}

HttpParserRequestLine::Action
HttpParserRequestLine::req_uri(HttpRequest &, u_char ch)
{
    if (is_usual(ch))
    {
        m_buff += ch;
        return PA_CONTINUE;
    }

    switch (ch)
    {
        case ' ':
            m_state = SW_HTTP_09;
            return PA_CONTINUE;
        case CR:
            m_minor = 9;
            m_state = SW_ALMOST_DONE;
            return PA_CONTINUE;
        case LF:
            m_minor = 9;
            return PA_DONE;
        case '#':
            m_buff += ch;
            return PA_CONTINUE;
        default:
            if (is_control(ch))
            {
                setError(error::bad_request);
                return PA_ERROR;
            }
            m_buff += ch;
            return PA_CONTINUE;
    }
}

HttpParserRequestLine::Action
HttpParserRequestLine::req_http_09(HttpRequest &, u_char ch)
{
    switch (ch)
    {
        case ' ':
            return PA_CONTINUE;
        case CR:
            m_minor = 9;
            m_state = SW_ALMOST_DONE;
            return PA_CONTINUE;
        case LF:
            m_minor = 9;
            return PA_DONE;
        case 'H':
            m_state = SW_HTTP_H;
            return PA_CONTINUE;
        default:
            setError(error::bad_request);
            return PA_ERROR;
    }
}

HttpParserRequestLine::Action
HttpParserRequestLine::req_http_H(HttpRequest &, u_char ch)
{
    if (ch != 'T')
    {
        setError(error::bad_request);
        return PA_ERROR;
    }
    m_state = SW_HTTP_HT;
    return PA_CONTINUE;
}

HttpParserRequestLine::Action
HttpParserRequestLine::req_http_HT(HttpRequest &, u_char ch)
{
    if (ch != 'T')
    {
        setError(error::bad_request);
        return PA_ERROR;
    }
    m_state = SW_HTTP_HTT;
    return PA_CONTINUE;
}

HttpParserRequestLine::Action
HttpParserRequestLine::req_http_HTT(HttpRequest &, u_char ch)
{
    if (ch != 'P')
    {
        setError(error::bad_request);
        return PA_ERROR;
    }
    m_state = SW_HTTP_HTTP;
    return PA_CONTINUE;
}

HttpParserRequestLine::Action
HttpParserRequestLine::req_http_HTTP(HttpRequest &, u_char ch)
{
    if (ch != '/')
    {
        setError(error::bad_request);
        return PA_ERROR;
    }
    m_state = SW_FIRST_MAJOR_DIGIT;
    return PA_CONTINUE;
}

HttpParserRequestLine::Action
HttpParserRequestLine::req_first_major_digit(HttpRequest &, u_char ch)
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

    m_state = SW_MAJOR_DIGIT;
    return PA_CONTINUE;
}

HttpParserRequestLine::Action
HttpParserRequestLine::req_major_digit(HttpRequest &, u_char ch)
{
    if (ch == '.')
    {
        m_state = SW_FIRST_MINOR_DIGIT;
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
HttpParserRequestLine::req_first_minor_digit(HttpRequest &, u_char ch)
{
    if (ch < '0' || ch > '9')
    {
        setError(error::bad_version);
        return PA_ERROR;
    }

    m_minor = ch - '0';
    m_state = SW_MINOR_DIGIT;
    return PA_CONTINUE;
}

HttpParserRequestLine::Action
HttpParserRequestLine::req_minor_digit(HttpRequest &, u_char ch)
{
    if (ch == CR)
    {
        m_state = SW_ALMOST_DONE;
        return PA_CONTINUE;
    }
    if (ch == LF)
        return PA_DONE;
    if (ch == ' ')
    {
        m_state = SW_SPACES_AFTER_DIGIT;
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
HttpParserRequestLine::req_spaces_after_digit(HttpRequest &, u_char ch)
{
    switch (ch)
    {
        case ' ':
            return PA_CONTINUE;
        case CR:
            m_state = SW_ALMOST_DONE;
            return PA_CONTINUE;
        case LF:
            return PA_DONE;
        default:
            setError(error::bad_request);
            return PA_ERROR;
    }
}

HttpParserRequestLine::Action
HttpParserRequestLine::req_almost_done(HttpRequest &, u_char ch)
{
    if (ch != LF)
    {
        setError(error::bad_line_ending);
        return PA_ERROR;
    }
    return PA_DONE;
}
