#include "HttpMessage.hpp"
#include "HttpParserState.hpp"
#include "ParserError.hpp"
#include <string>

void HttpParserState::process_header_line()
{
    m_state = 0;
    if (*m_buff.rbegin() == ' ')
    {
        size_t i = m_buff.size();
        while (i > 0 && m_buff[i - 1] == ' ')
            i--;
        m_buff.resize(i);
    }
    m_message.setHeader(
        m_buff.substr(0, m_chunk_size), m_buff.substr(m_chunk_size)
    );
    m_buff.clear();
}

void HttpParserState::process_transfer_encoding()
{
    HttpMessage::Headers::const_iterator it1 =
        m_message.getHeader("transfer-encoding");

    if (it1 == m_message.headers().end())
    {
        m_chunked = false;
        return;
    }
    if (it1->second == "chunked")
        m_chunked = true;
    else if (it1->second == "identity")
        m_chunked = false;
    else
        setError(error::unsupported_transfer);
}

void HttpParserState::process_content_length()
{

    // int count = request.headers().count("content-length");
    //
    // if (count == 0)
    // {
    //     if (m_discard_body)
    //         return request.setComplete(true);
    //     else if (!m_chunked)
    //         return setError(error::bad_request);
    //     return;
    // }
    // if (count > 1)
    //     return setError(error::multiple_content_length);
    //
    // HttpMessage::Headers::const_iterator it =
    //     request.getHeader("content-length");
    // ssize_t            content_length;
    // std::istringstream iss(it->second);
    //
    // iss >> content_length;
    // if (iss.bad() || !iss.eof())
    // {
    //     setError(error::bad_content_length);
    //     return;
    // }
    // request.setContentLength(content_length);
}

void HttpParserState::process_headers()
{
}

void HttpParserState::process_host()
{
    HttpMessage::Headers                 headers = m_message.headers();
    HttpMessage::Headers::const_iterator it      = headers.find("host");

    if (it == headers.end() || it->second.empty())
        setError(error::bad_request);
}
