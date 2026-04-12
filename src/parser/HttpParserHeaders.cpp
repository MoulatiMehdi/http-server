#include "HttpParserHeaders.hpp"
#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include "ParserError.hpp"
#include <iostream>
#include <sstream>
#include <string>

const HttpParserHeaders::Handler HttpParserHeaders::handlers[] = {
    &HttpParserHeaders::hdr_start,
    &HttpParserHeaders::hdr_name,
    &HttpParserHeaders::hdr_space_before_value,
    &HttpParserHeaders::hdr_value,
    &HttpParserHeaders::hdr_almost_done,
    &HttpParserHeaders::hdr_header_almost_done,
};

HttpParserHeaders::HttpParserHeaders()
    : HttpParserState(),
      m_header_name(),
      m_header_value()
{
}

void HttpParserHeaders::processHeaderLine(HttpRequest &request)
{
    if (!m_header_value.empty())
    {
        size_t i = m_header_value.size() - 1;
        if (m_header_value.back() == ' ')
        {
            while (i > 0 && m_header_value[i] == ' ')
                i--;
            i++;
            m_header_value.erase(i, m_header_value.size() - i);
        }
    }
    request.setHeader(m_header_name, m_header_value);
}

void HttpParserHeaders::handle_transfer_encoding(HttpRequest &request)
{
    HttpRequest::Headers::const_iterator it1 =
        request.getHeader("transfer-encoding");

    if (it1 == request.headers().end())
    {
        m_chunked = false;
        return;
    }
    if (request.version() == 9)
        return setError(ParserError::bad_request);

    if (it1->second == "chunked")
        m_chunked = true;
    else if (it1->second == "identity")
        m_chunked = false;
    else
        setError(ParserError::unsupported_transfer);
}

void HttpParserHeaders::handle_content_length(HttpRequest &request)
{

    int count = request.headers().count("content-length");

    if (count == 0)
        return setError(ParserError::bad_request);
    if (count > 1)
        return setError(ParserError::multiple_content_length);

    HttpRequest::Headers::const_iterator it =
        request.getHeader("content-length");
    size_t             content_length;
    std::istringstream iss(it->second);

    iss >> content_length;
    if (iss.bad() || !iss.eof())
    {
        setError(ParserError::bad_content_length);
        return;
    }
    request.setContentLength(content_length);
    if (content_length == 0)
        request.setComplete(true);
}

void HttpParserHeaders::clear()
{
    m_header_value.clear();
    m_header_value.clear();
}

void HttpParserHeaders::processHeaders(HttpRequest &request)
{
    handle_transfer_encoding(request);
    if (!m_chunked)
        handle_content_length(request);
}
