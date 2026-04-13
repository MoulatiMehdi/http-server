#include "HttpParserRequestLine.hpp"
#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include "Method.hpp"
#include "ParserError.hpp"

HttpParserRequestLine::Handler HttpParserRequestLine::handlers[] = {
    &HttpParserRequestLine::req_start,
    &HttpParserRequestLine::req_method,
    &HttpParserRequestLine::req_spaces_before_uri,
    &HttpParserRequestLine::req_uri_after_slash,
    &HttpParserRequestLine::req_check_uri,
    &HttpParserRequestLine::req_uri,
    &HttpParserRequestLine::req_http_09,
    &HttpParserRequestLine::req_http_H,
    &HttpParserRequestLine::req_http_HT,
    &HttpParserRequestLine::req_http_HTT,
    &HttpParserRequestLine::req_http_HTTP,
    &HttpParserRequestLine::req_first_major_digit,
    &HttpParserRequestLine::req_major_digit,
    &HttpParserRequestLine::req_first_minor_digit,
    &HttpParserRequestLine::req_minor_digit,
    &HttpParserRequestLine::req_spaces_after_digit,
    &HttpParserRequestLine::req_almost_done,
};

HttpParserRequestLine::HttpParserRequestLine() : HttpParserState()
{
}

void HttpParserRequestLine::processRequestLine(HttpRequest &request)
{
    request.setUri(m_buff);
    m_buff.clear();
    request.setVersion(m_major, m_minor);
    if (request.version() == 9)
    {
        setError(error::unsupported_version);
        return;
    }
    m_discard_body = request.method() != method::POST;
}

void HttpParserRequestLine::parseRequestLine(HttpRequest &request, Buffer &buff)
{
    while (!buff.empty())
    {
        char   ch     = buff.getc();
        Action action = (this->*handlers[m_state])(request, ch);

        switch (action)
        {
            case PA_ERROR:
                processError(request);
                return;
            case PA_DONE:
                processRequestLine(request);
                m_phase = P_HEADERS;
                m_state = 0;
                return;
            case PA_CONTINUE:
            case PA_OK:
                break;
        }
    }
}

HttpParserRequestLine::~HttpParserRequestLine()
{
}
