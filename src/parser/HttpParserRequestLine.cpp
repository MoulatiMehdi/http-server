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

HttpParserRequestLine::HttpParserRequestLine()
    : HttpParserState(),
      m_major(0),
      m_minor(0),
      m_method(),
      m_target()
{
}

void HttpParserRequestLine::processRequestLine(HttpRequest &request)
{
    Method method = string_to_method(m_method);

    if (method == Method::UNKNOWN)
    {
        setError(ParserError::unsupported_method);
        return;
    }

    request.setMethod(method);
    request.setUri(m_target);
    request.setVersion(m_major, m_minor);
    if (request.version() == 9)
    {
        setError(ParserError::unsupported_version);
        return;
    }
    m_discard_body = request.method() != Method::POST;
}

void HttpParserRequestLine::clear()
{
    m_method.clear();
    m_target.clear();
    m_minor = 0;
    m_major = 0;
}

HttpParserRequestLine::~HttpParserRequestLine()
{
}
