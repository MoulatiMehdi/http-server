#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestParser.hpp"
#include "Logger.hpp"
#include "Method.hpp"
#include "ParserError.hpp"
#include "RouteResult.hpp"
#include "Router.hpp"
#include "Status.hpp"

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
        return RES_HDR_CONTINUE;

    if (!is_valid_method_char(ch))
    {
        setError(error::bad_method);
        return RES_HDR_ERROR;
    }

    m_buff  += ch;
    m_state  = SW_METHOD;
    return RES_HDR_CONTINUE;
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
        return RES_HDR_ERROR;
    }
    m_buff += ch;
    return RES_HDR_CONTINUE;
}

unsigned int HttpParserState::req_spaces_before_uri(u_char ch)
{
    if (ch == '/')
    {
        m_buff  += ch;
        m_state  = SW_URI_AFTER_SLASH;
        return RES_HDR_CONTINUE;
    }

    if (ch != ' ')
    {
        setError(error::bad_request);
        return RES_HDR_ERROR;
    }
    return RES_HDR_CONTINUE;
}

unsigned int HttpParserState::req_uri_after_slash(u_char ch)
{
    if (is_usual(ch))
    {
        m_buff  += ch;
        m_state  = SW_CHECK_URI;
        return RES_HDR_CONTINUE;
    }

    switch (ch)
    {
        case ' ':
            m_state = SW_HTTP_09;
            return RES_URI_DONE;
        case CR:
            m_minor = 9;
            m_major = 0;
            m_state = SW_ALMOST_DONE;
            return RES_HDR_CONTINUE;
        case LF:
            m_minor = 9;
            m_major = 0;
            return RES_REQUEST_LINE_DONE;
        case '.':
        case '%':
        case '/':
        case '?':
        case '#':
            m_buff  += ch;
            m_state  = SW_URI;
            return RES_HDR_CONTINUE;
        case '+':
            m_buff += ch;
            return RES_HDR_CONTINUE;
        default:
            if (is_control(ch))
            {
                setError(error::bad_request);
                return RES_HDR_ERROR;
            }
            m_buff  += ch;
            m_state  = SW_CHECK_URI;
            return RES_HDR_CONTINUE;
    }
}

unsigned int HttpParserState::req_check_uri(u_char ch)
{
    if (is_usual(ch))
    {
        m_buff += ch;
        return RES_HDR_CONTINUE;
    }

    switch (ch)
    {
        case '/':
            m_buff  += ch;
            m_state  = SW_URI_AFTER_SLASH;
            return RES_HDR_CONTINUE;
        case '.':
            m_buff += ch;
            return RES_HDR_CONTINUE;
        case ' ':
            m_state = SW_HTTP_09;
            return RES_URI_DONE;
        case CR:
            m_minor = 9;
            m_state = SW_ALMOST_DONE;
            return RES_HDR_CONTINUE;
        case LF:
            m_minor = 9;
            return RES_REQUEST_LINE_DONE;
        case '%':
        case '?':
        case '#':
            m_buff  += ch;
            m_state  = SW_URI;
            return RES_HDR_CONTINUE;
        case '+':
            m_buff += ch;
            return RES_HDR_CONTINUE;
        default:
            if (is_control(ch))
            {
                setError(error::bad_request);
                return RES_HDR_ERROR;
            }
            m_buff += ch;
            return RES_HDR_CONTINUE;
    }
}

unsigned int HttpParserState::req_uri(u_char ch)
{
    if (is_usual(ch))
    {
        m_buff += ch;
        return RES_HDR_CONTINUE;
    }

    switch (ch)
    {
        case ' ':
            m_state = SW_HTTP_09;
            return RES_URI_DONE;
        case CR:
            m_minor = 9;
            m_state = SW_ALMOST_DONE;
            return RES_HDR_CONTINUE;
        case LF:
            m_minor = 9;
            return RES_REQUEST_LINE_DONE;
        case '#':
            m_buff += ch;
            return RES_HDR_CONTINUE;
        default:
            if (is_control(ch))
            {
                setError(error::bad_request);
                return RES_HDR_ERROR;
            }
            m_buff += ch;
            return RES_HDR_CONTINUE;
    }
}

unsigned int HttpParserState::req_http_09(u_char ch)
{
    switch (ch)
    {
        case ' ':
            return RES_HDR_CONTINUE;
        case CR:
            m_minor = 9;
            m_state = SW_ALMOST_DONE;
            return RES_HDR_CONTINUE;
        case LF:
            m_minor = 9;
            return RES_REQUEST_LINE_DONE;
        case 'H':
            m_state = SW_HTTP_H;
            return RES_HDR_CONTINUE;
        default:
            setError(error::bad_request);
            return RES_HDR_ERROR;
    }
}

unsigned int HttpParserState::req_http_H(u_char ch)
{
    if (ch != 'T')
    {
        setError(error::bad_request);
        return RES_HDR_ERROR;
    }
    m_state = SW_HTTP_HT;
    return RES_HDR_CONTINUE;
}

unsigned int HttpParserState::req_http_HT(u_char ch)
{
    if (ch != 'T')
    {
        setError(error::bad_request);
        return RES_HDR_ERROR;
    }
    m_state = SW_HTTP_HTT;
    return RES_HDR_CONTINUE;
}

unsigned int HttpParserState::req_http_HTT(u_char ch)
{
    if (ch != 'P')
    {
        setError(error::bad_request);
        return RES_HDR_ERROR;
    }
    m_state = SW_HTTP_HTTP;
    return RES_HDR_CONTINUE;
}

unsigned int HttpParserState::req_http_HTTP(u_char ch)
{
    if (ch != '/')
    {
        setError(error::bad_request);
        return RES_HDR_ERROR;
    }
    m_state = SW_FIRST_MAJOR_DIGIT;
    return RES_HDR_CONTINUE;
}

unsigned int HttpParserState::req_first_major_digit(u_char ch)
{
    if (ch < '1' || ch > '9')
    {
        setError(error::bad_request);
        return RES_HDR_ERROR;
    }

    m_major = ch - '0';

    if (m_major > 1)
    {
        setError(error::unsupported_version);
        return RES_HDR_ERROR;
    }

    m_state = SW_MAJOR_DIGIT;
    return RES_HDR_CONTINUE;
}

unsigned int HttpParserState::req_major_digit(u_char ch)
{
    if (ch == '.')
    {
        m_state = SW_FIRST_MINOR_DIGIT;
        return RES_HDR_CONTINUE;
    }

    if (ch < '0' || ch > '9')
    {
        setError(error::bad_request);
        return RES_HDR_ERROR;
    }

    m_major = m_major * 10 + (ch - '0');

    if (m_major > 1)
    {
        setError(error::unsupported_version);
        return RES_HDR_ERROR;
    }
    return RES_HDR_CONTINUE;
}

unsigned int HttpParserState::req_first_minor_digit(u_char ch)
{
    if (ch < '0' || ch > '9')
    {
        setError(error::bad_version);
        return RES_HDR_ERROR;
    }

    m_minor = ch - '0';
    m_state = SW_MINOR_DIGIT;
    return RES_HDR_CONTINUE;
}

unsigned int HttpParserState::req_minor_digit(u_char ch)
{
    if (ch == CR)
    {
        m_state = SW_ALMOST_DONE;
        return RES_VERSION_DONE;
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
        return RES_HDR_ERROR;
    }

    m_minor = m_minor * 10 + (ch - '0');
    if (m_minor > 999)
    {
        setError(error::bad_request);
        return RES_HDR_ERROR;
    }

    return RES_HDR_CONTINUE;
}

unsigned int HttpParserState::req_spaces_after_digit(u_char ch)
{
    switch (ch)
    {
        case ' ':
            return RES_HDR_CONTINUE;
        case CR:
            m_state = SW_ALMOST_DONE;
            return RES_HDR_CONTINUE;
        case LF:
            return RES_REQUEST_LINE_DONE;
        default:
            setError(error::bad_request);
            return RES_HDR_ERROR;
    }
}

unsigned int HttpParserState::req_almost_done(u_char ch)
{
    if (ch != LF)
    {
        setError(error::bad_line_ending);
        return RES_HDR_ERROR;
    }
    return RES_REQUEST_LINE_DONE;
}

void HttpRequestParser::parse_request_line(Buffer &buff)
{
    const static HttpRequestParser::Handler handlers[] = {
        &HttpRequestParser::req_start,
        &HttpRequestParser::req_method,
        &HttpRequestParser::req_spaces_before_uri,
        &HttpRequestParser::req_uri_after_slash,
        &HttpRequestParser::req_check_uri,
        &HttpRequestParser::req_uri,
        &HttpRequestParser::req_http_09,
        &HttpRequestParser::req_http_H,
        &HttpRequestParser::req_http_HT,
        &HttpRequestParser::req_http_HTT,
        &HttpRequestParser::req_http_HTTP,
        &HttpRequestParser::req_first_major_digit,
        &HttpRequestParser::req_major_digit,
        &HttpRequestParser::req_first_minor_digit,
        &HttpRequestParser::req_minor_digit,
        &HttpRequestParser::req_spaces_after_digit,
        &HttpRequestParser::req_almost_done,
    };
    while (!buff.empty())
    {
        char ch = buff.getc();
        m_parsed++;
        if (m_parsed > MAX_REQUEST_BUFFER)
        {
            setError(error::url_too_large);
            return;
        }
        unsigned int action = (this->*handlers[m_state])(ch);
        switch (static_cast<RequestLineResult>(action))
        {
            case RES_ERROR:
                return;
            case RES_CONTINUE:
                break;
            case RES_METHOD_DONE:
                m_request.setMethod(m_buff);
                if (m_request.method() == method::UNKNOWN)
                    return setError(error::unsupported_method);
                m_discard_body = m_request.method() != method::POST;
                m_buff.clear();
                break;
            case RES_URI_DONE:
                m_request.setUri(m_buff);
                if (!m_request.uri().isvalid())
                    return setError(error::bad_request);
                m_buff.clear();
                break;
            case RES_VERSION_DONE:
                m_request.setVersion(m_major, m_minor);
                if (m_request.version() == 9)
                    return setError(error::unsupported_version);
                break;
            case RES_REQUEST_LINE_DONE:
                Logger::info(m_request.to_string());
                route = Router::resolve(m_request.m_server_config, m_request);
                if(route.location == NULL)
                    m_request.setMaxBodySize(route.server->client_max_body_size);
                else 
                    m_request.setMaxBodySize(route.location->client_max_body_size);
                Router::printRouteResult(route);
                m_parsed = 0;
                m_phase  = PHASE_HEADERS;
                m_state  = 0;
                if (m_request.good())
                {
                    switch (route.action)
                    {
                        case ROUTE_CGI:
                            break;
                        case ROUTE_STATIC_FILE:
                            break;
                        case ROUTE_DIRECTORY_LISTING:
                            break;
                        case ROUTE_UPLOAD:
                            break;
                        case ROUTE_REDIRECT:
                            m_request.setStatus(route.statusCode);
                            return m_request.setComplete(true);
                        case ROUTE_DELETE:
                            break;
                        case ROUTE_ERROR:
                            m_request.setStatus(route.statusCode);
                            return;
                    }
                }
                return;
        }
    }
}
