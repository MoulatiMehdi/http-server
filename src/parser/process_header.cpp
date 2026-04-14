#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include "ParserError.hpp"
#include <iostream>
#include <sstream>
#include <string>

void HttpParserState::process_header_line(HttpRequest &request)
{
    m_state = 0;
    if (*m_buff.rbegin() == ' ')
    {
        size_t i = m_buff.size();
        while (i > 0 && m_buff[i - 1] == ' ')
            i--;
        m_buff.resize(i);
    }
    request.setHeader(
        m_buff.substr(0, m_chunk_size), m_buff.substr(m_chunk_size)
    );
    m_buff.clear();
}

void HttpParserState::process_transfer_encoding(HttpRequest &request)
{
    HttpRequest::Headers::const_iterator it1 =
        request.getHeader("transfer-encoding");

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
        setError(error::unsupported_transfer);
}

void HttpParserState::process_content_length(HttpRequest &request)
{

    int count = request.headers().count("content-length");

    if (count == 0)
        return setError(error::bad_request);
    if (count > 1)
        return setError(error::multiple_content_length);

    HttpRequest::Headers::const_iterator it =
        request.getHeader("content-length");
    ssize_t            content_length;
    std::istringstream iss(it->second);

    iss >> content_length;
    if (iss.bad() || !iss.eof())
    {
        setError(error::bad_content_length);
        return;
    }
    request.setContentLength(content_length);
}

void HttpParserState::process_headers(HttpRequest &request)
{
    m_phase = P_BODY;
    m_state = 0;
    process_transfer_encoding(request);
    if (!m_chunked)
        process_content_length(request);
    if (request.method() == method::POST)
    {
        if (request.body().open_file() < 0)
            return setError(error::bad_request);
    }
}
