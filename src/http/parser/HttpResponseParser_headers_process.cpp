
#include "HttpResponse.hpp"
#include "HttpResponseParser.hpp"
#include "Logger.hpp"
#include "ParserError.hpp"
#include "Status.hpp"
#include <cstddef>
#include <iostream>
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

    m_response.setHeader(name, m_buff.substr(m_chunk_size));
    m_buff.clear();
}

void HttpResponseParser::process_headers()
{
    process_content_length();
    process_status();
}

void HttpResponseParser::process_content_length()
{
    int count = m_response.headers().count("content-length");

    if (count > 1)
        return setError(error::multiple_content_length);

    HttpMessage::Headers::const_iterator it =
        m_response.getHeader("content-length");
    ssize_t            content_length;
    std::istringstream iss(it->second);

    iss >> content_length;
    if (iss.bad() || !iss.eof())
    {
        setError(error::bad_content_length);
        return;
    }
    m_response.setContentLength(content_length);
}

void HttpResponseParser::process_status()
{
    HttpMessage::Headers::const_iterator it = m_response.getHeader("status");

    if (it == m_response.headers().end())
    {
        Logger::info("Status:  not found");
        m_response.setStatus(status::OK);
        return;
    }
    std::istringstream iss(it->second);

    iss >> m_code;
    if (iss.bad())
    {
        Logger::info("Status : " + it->second + ": bad status");
        setError(error::bad_status);
        return;
    }
    Logger::info("Status :" + it->second);
    if (m_code > 599 || m_code < 100)
        setError(error::bad_status);
}

void HttpResponseParser::process_error()
{
    switch (m_error)
    {
        case error::ok:
            return m_response.setStatus(status::OK);
        default:
            return m_response.setStatus(status::BAD_GATEWAY);
            break;
    }
}
