
#include "HttpResponse.hpp"
#include "HttpResponseParser.hpp"
#include <cstddef>
#include <sstream>
#include <string>

void HttpResponseParser::process_header_line()
{
    m_state = 0;
    if (*m_buff.rbegin() == ' ')
    {
        size_t i = m_buff.size();
        while (i > 0 && m_buff[i - 1] == ' ')
            i--;
        m_buff.resize(i);
    }

    std::string name = m_buff.substr(0, m_chunk_size);

    response.setHeader(name, m_buff.substr(m_chunk_size));
    m_buff.clear();
}

void HttpResponseParser::process_headers()
{
    process_content_length();
    process_status();
}

void HttpResponseParser::process_content_length()
{
    int count = response.headers().count("content-length");

    if (count > 1)
        return setError(error::multiple_content_length);

    HttpMessage::Headers::const_iterator it =
        response.getHeader("content-length");
    ssize_t            content_length;
    std::istringstream iss(it->second);

    iss >> content_length;
    if (iss.bad() || !iss.eof())
    {
        setError(error::bad_content_length);
        return;
    }
    response.setContentLength(content_length);
}

void HttpResponseParser::process_status()
{
    HttpMessage::Headers::const_iterator it = response.getHeader("status");

    if (it == response.headers().end())
    {
        response.setStatus(status::OK);
        return;
    }
    ssize_t            content_length;
    std::istringstream iss(it->second);

    iss >> std::noskipws >> content_length;
    if (iss.bad() || !iss.eof())
    {
        setError(error::bad_status);
        return;
    }
    response.setContentLength(content_length);
}

void HttpResponseParser::process_error()
{
    switch (m_error)
    {
        case error::ok:
            return response.setStatus(status::OK);
        default:
            return response.setStatus(status::BAD_GATEWAY);
            break;
    }
}
