
#include "HttpResponseParser.hpp"
#include <sstream>

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
    request.setHeader(
        m_buff.substr(0, m_chunk_size), m_buff.substr(m_chunk_size)
    );
    m_buff.clear();
}

void HttpResponseParser::process_headers()
{
    process_content_length();
    process_status();
    process_remove_headers();
}

void HttpResponseParser::process_content_length()
{
    int count = request.headers().count("content-length");

    if (count > 1)
        return setError(error::multiple_content_length);

    HttpMessage::Headers::const_iterator it =
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

void HttpResponseParser::process_status()
{
    HttpMessage::Headers::const_iterator it = request.getHeader("status");

    if (it == request.headers().end())
    {
        request.setStatus(status::OK);
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
    request.setContentLength(content_length);
}

void HttpResponseParser::process_error()
{
    switch (m_error)
    {
        case error::ok:
            return request.setStatus(status::OK);
        default:
            return request.setStatus(status::BAD_GATEWAY);
            break;
    }
}

void HttpResponseParser::process_remove_headers()
{
    const std::string    names[] = {"transfer-encoding"};
    const size_t         size    = sizeof(names) / sizeof(names[0]);
    HttpMessage::Headers headers = request.headers();

    for (size_t i = 0; i < size; i++)
    {
        HttpMessage::Headers::iterator it = headers.find(names[i]);
        if (it != headers.end())
            headers.erase(it);
    }
}
