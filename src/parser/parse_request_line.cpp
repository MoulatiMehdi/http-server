#include "HttpParserState.hpp"

enum RequestLineState
{
    SW_START = 0,
    SW_METHOD,
    SW_SPACES_BEFORE_URI,
    SW_URI_AFTER_SLASH,
    SW_CHECK_URI,
    SW_URI,
    SW_HTTP_09,
    SW_HTTP_H,
    SW_HTTP_HT,
    SW_HTTP_HTT,
    SW_HTTP_HTTP,
    SW_FIRST_MAJOR_DIGIT,
    SW_MAJOR_DIGIT,
    SW_FIRST_MINOR_DIGIT,
    SW_MINOR_DIGIT,
    SW_SPACES_AFTER_DIGIT,
    SW_ALMOST_DONE,
};

enum RequestLineResult
{
    RES_ERROR,
    RES_CONTINUE,
    RES_METHOD_DONE,
    RES_URI_DONE,
    RES_VERSION_DONE,
    RES_REQUEST_LINE_DONE
};

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

unsigned int HttpParserState::req_start(u_char ch)
{
    if (ch == CR || ch == LF)
        return RES_CONTINUE;

    if (!is_valid_method_char(ch))
    {
        setError(error::bad_method);
        return RES_ERROR;
    }

    m_buff  += ch;
    m_state  = SW_METHOD;
    return RES_CONTINUE;
}

unsigned int HttpParserState::req_method(u_char ch)
{
    if (ch == ' ')
    {
        m_state = SW_SPACES_BEFORE_URI;
        return RES_METHOD_DONE;
    }

    if (!is_valid_method_char(ch))
    {
        setError(error::bad_method);
        return RES_ERROR;
    }
    m_buff += ch;
    return RES_CONTINUE;
}

unsigned int HttpParserState::req_spaces_before_uri(u_char ch)
{
    if (ch == '/')
    {
        m_buff  += ch;
        m_state  = SW_URI_AFTER_SLASH;
        return RES_CONTINUE;
    }

    if (ch != ' ')
    {
        setError(error::bad_request);
        return RES_ERROR;
    }
    return RES_CONTINUE;
}

unsigned int HttpParserState::req_uri_after_slash(u_char ch)
{
    if (is_usual(ch))
    {
        m_buff  += ch;
        m_state  = SW_CHECK_URI;
        return RES_CONTINUE;
    }

    switch (ch)
    {
        case ' ':
            m_state = SW_HTTP_09;
            return RES_URI_DONE;
        case CR:
            m_minor = 9;
            m_state = SW_ALMOST_DONE;
            return RES_CONTINUE;
        case LF:
            m_minor = 9;
            return RES_REQUEST_LINE_DONE;
        case '.':
        case '%':
        case '/':
        case '?':
        case '#':
            m_buff  += ch;
            m_state  = SW_URI;
            return RES_CONTINUE;
        case '+':
            m_buff += ch;
            return RES_CONTINUE;
        default:
            if (is_control(ch))
            {
                setError(error::bad_request);
                return RES_ERROR;
            }
            m_buff  += ch;
            m_state  = SW_CHECK_URI;
            return RES_CONTINUE;
    }
}

unsigned int HttpParserState::req_check_uri(u_char ch)
{
    if (is_usual(ch))
    {
        m_buff += ch;
        return RES_CONTINUE;
    }

    switch (ch)
    {
        case '/':
            m_buff  += ch;
            m_state  = SW_URI_AFTER_SLASH;
            return RES_CONTINUE;
        case '.':
            m_buff += ch;
            return RES_CONTINUE;
        case ' ':
            m_state = SW_HTTP_09;
            return RES_URI_DONE;
        case CR:
            m_minor = 9;
            m_state = SW_ALMOST_DONE;
            return RES_CONTINUE;
        case LF:
            m_minor = 9;
            return RES_REQUEST_LINE_DONE;
        case '%':
        case '?':
        case '#':
            m_buff  += ch;
            m_state  = SW_URI;
            return RES_CONTINUE;
        case '+':
            m_buff += ch;
            return RES_CONTINUE;
        default:
            if (is_control(ch))
            {
                setError(error::bad_request);
                return RES_ERROR;
            }
            m_buff += ch;
            return RES_CONTINUE;
    }
}

unsigned int HttpParserState::req_uri(u_char ch)
{
    if (is_usual(ch))
    {
        m_buff += ch;
        return RES_CONTINUE;
    }

    switch (ch)
    {
        case ' ':
            m_state = SW_HTTP_09;
            return RES_URI_DONE;
        case CR:
            m_minor = 9;
            m_state = SW_ALMOST_DONE;
            return RES_CONTINUE;
        case LF:
            m_minor = 9;
            return RES_REQUEST_LINE_DONE;
        case '#':
            m_buff += ch;
            return RES_CONTINUE;
        default:
            if (is_control(ch))
            {
                setError(error::bad_request);
                return RES_ERROR;
            }
            m_buff += ch;
            return RES_CONTINUE;
    }
}

unsigned int HttpParserState::req_http_09(u_char ch)
{
    switch (ch)
    {
        case ' ':
            return RES_CONTINUE;
        case CR:
            m_minor = 9;
            m_state = SW_ALMOST_DONE;
            return RES_CONTINUE;
        case LF:
            m_minor = 9;
            return RES_REQUEST_LINE_DONE;
        case 'H':
            m_state = SW_HTTP_H;
            return RES_CONTINUE;
        default:
            setError(error::bad_request);
            return RES_ERROR;
    }
}

unsigned int HttpParserState::req_http_H(u_char ch)
{
    if (ch != 'T')
    {
        setError(error::bad_request);
        return RES_ERROR;
    }
    m_state = SW_HTTP_HT;
    return RES_CONTINUE;
}

unsigned int HttpParserState::req_http_HT(u_char ch)
{
    if (ch != 'T')
    {
        setError(error::bad_request);
        return RES_ERROR;
    }
    m_state = SW_HTTP_HTT;
    return RES_CONTINUE;
}

unsigned int HttpParserState::req_http_HTT(u_char ch)
{
    if (ch != 'P')
    {
        setError(error::bad_request);
        return RES_ERROR;
    }
    m_state = SW_HTTP_HTTP;
    return RES_CONTINUE;
}

unsigned int HttpParserState::req_http_HTTP(u_char ch)
{
    if (ch != '/')
    {
        setError(error::bad_request);
        return RES_ERROR;
    }
    m_state = SW_FIRST_MAJOR_DIGIT;
    return RES_CONTINUE;
}

unsigned int HttpParserState::req_first_major_digit(u_char ch)
{
    if (ch < '1' || ch > '9')
    {
        setError(error::bad_request);
        return RES_ERROR;
    }

    m_major = ch - '0';

    if (m_major > 1)
    {
        setError(error::bad_version);
        return RES_ERROR;
    }

    m_state = SW_MAJOR_DIGIT;
    return RES_CONTINUE;
}

unsigned int HttpParserState::req_major_digit(u_char ch)
{
    if (ch == '.')
    {
        m_state = SW_FIRST_MINOR_DIGIT;
        return RES_CONTINUE;
    }

    if (ch < '0' || ch > '9')
    {
        setError(error::bad_request);
        return RES_ERROR;
    }

    m_major = m_major * 10 + (ch - '0');

    if (m_major > 1)
    {
        setError(error::bad_version);
        return RES_ERROR;
    }
    return RES_CONTINUE;
}

unsigned int HttpParserState::req_first_minor_digit(u_char ch)
{
    if (ch < '0' || ch > '9')
    {
        setError(error::bad_version);
        return RES_ERROR;
    }

    m_minor = ch - '0';
    m_state = SW_MINOR_DIGIT;
    return RES_CONTINUE;
}

unsigned int HttpParserState::req_minor_digit(u_char ch)
{
    if (ch == CR)
    {
        m_state = SW_ALMOST_DONE;
        return RES_CONTINUE;
    }
    if (ch == LF)
        return RES_REQUEST_LINE_DONE;
    if (ch == ' ')
    {
        m_state = SW_SPACES_AFTER_DIGIT;
        return RES_VERSION_DONE;
    }

    if (ch < '0' || ch > '9')
    {
        setError(error::bad_request);
        return RES_ERROR;
    }

    if (m_minor > 99)
    {
        setError(error::bad_request);
        return RES_ERROR;
    }

    m_minor = m_minor * 10 + (ch - '0');
    return RES_CONTINUE;
}

unsigned int HttpParserState::req_spaces_after_digit(u_char ch)
{
    switch (ch)
    {
        case ' ':
            return RES_CONTINUE;
        case CR:
            m_state = SW_ALMOST_DONE;
            return RES_CONTINUE;
        case LF:
            return RES_REQUEST_LINE_DONE;
        default:
            setError(error::bad_request);
            return RES_ERROR;
    }
}

unsigned int HttpParserState::req_almost_done(u_char ch)
{
    if (ch != LF)
    {
        setError(error::bad_line_ending);
        return RES_ERROR;
    }
    return RES_REQUEST_LINE_DONE;
}

void HttpParserState::process_request_line(unsigned int result)
{
    switch (static_cast<RequestLineResult>(result))
    {
        case RES_ERROR:
        case RES_CONTINUE:
            break;
        case RES_METHOD_DONE:
            request.setMethod(m_buff);
            if (request.method() == method::UNKNOWN)
                setError(error::unsupported_method);
            m_buff.clear();
            return;
        case RES_URI_DONE:
            request.setUri(m_buff);
            m_buff.clear();
            return;
        case RES_VERSION_DONE:
            request.setVersion(m_major, m_minor);
            if (request.version() == 9)
                setError(error::unsupported_version);
            return;
        case RES_REQUEST_LINE_DONE:
            m_phase = P_HEADERS;
            m_state = 0;
            return;
    }
}

void HttpParserState::parse_request_line(Buffer &buff)
{
    const static HttpParserState::Handler handlers[] = {
        &HttpParserState::req_start,
        &HttpParserState::req_method,
        &HttpParserState::req_spaces_before_uri,
        &HttpParserState::req_uri_after_slash,
        &HttpParserState::req_check_uri,
        &HttpParserState::req_uri,
        &HttpParserState::req_http_09,
        &HttpParserState::req_http_H,
        &HttpParserState::req_http_HT,
        &HttpParserState::req_http_HTT,
        &HttpParserState::req_http_HTTP,
        &HttpParserState::req_first_major_digit,
        &HttpParserState::req_major_digit,
        &HttpParserState::req_first_minor_digit,
        &HttpParserState::req_minor_digit,
        &HttpParserState::req_spaces_after_digit,
        &HttpParserState::req_almost_done,
    };
    while (!buff.empty())
    {
        char         ch     = buff.getc();
        unsigned int action = (this->*handlers[m_state])(ch);
        process_request_line(action);
    }
}
