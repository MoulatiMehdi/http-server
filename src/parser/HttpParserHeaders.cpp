#include "HttpParserHeaders.hpp"
#include "Error.hpp"
#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include <iostream>
#include <sstream>
#include <string>

#define MAX_BODY_SIZE 4096

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
    request.set(m_header_name, m_header_value);
}

void HttpParserHeaders::handle_transfer_encoding(HttpRequest &request)
{
    HttpRequest::Headers::const_iterator it1 = request.get("transfer-encoding");

    if (it1 == request.headers().end())
    {
        m_chunked = false;
        return;
    }
    if (request.version() == 9)
        return setError(error::bad_request);

    if (it1->second == "chunked")
        m_chunked = true;
    else if (it1->second == "identity")
        m_chunked = false;
    else
        setError(Error::unsupported_transfer);
}

void HttpParserHeaders::handle_content_length(HttpRequest &request)
{
    HttpRequest::Headers::const_iterator it2 = request.get("content-length");

    if (it2 == request.headers().end())
        return setError(error::bad_request);

    size_t             content_length;
    std::istringstream iss(it2->second);

    iss >> content_length;
    if (iss.bad() || !iss.eof())
    {
        setError(Error::bad_content_length);
        return;
    }
    request.setContentLength(content_length);
}

void HttpParserHeaders::processHeaders(HttpRequest &request)
{
    handle_transfer_encoding(request);
    if (!m_chunked)
        handle_content_length(request);
}
